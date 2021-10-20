#include <cassert>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <iostream>
#include <stdint.h>
#include <limits>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const uint64_t kMaxUint64 = std::numeric_limits<uint64_t>::max();

struct HistogramData {
  double median;
  double percentile95;
  double percentile99;
  double average;
  double standard_deviation;
  // zero-initialize new members since old Statistics::histogramData()
  // implementations won't write them.
  double max = 0.0;
  uint64_t count = 0;
  uint64_t sum = 0;
  double min = 0.0;
};

class HistogramBucketMapper {
public:
  // constructor
  HistogramBucketMapper() {
    // If you change this, you also need to change
    // size of array buckets_ in HistogramImpl
    // so this is fixed to be size==109
    bucketValues_ = {1, 2};
    double bucket_val = static_cast<double>(bucketValues_.back());
    while ((bucket_val = 1.5 * bucket_val) <= static_cast<double>(kMaxUint64)) {
      bucketValues_.push_back(static_cast<uint64_t>(bucket_val));
      // 只保留最大的两位
      uint64_t pow_of_ten = 1;
      while (bucketValues_.back() / 10 > 10) {
        bucketValues_.back() /= 10;
        pow_of_ten *= 10;
      }
      bucketValues_.back() *= pow_of_ten;
    }
    maxBucketValue_ = bucketValues_.back();
    minBucketValue_ = bucketValues_.front();
  }

  // converts a value to the bucket index
  size_t IndexForValue(uint64_t value) const {
    auto beg = bucketValues_.begin();
    auto end = bucketValues_.end();
    // 这为什么要-1呢 这样感觉不是很统一啊
    // 跟底下这个else相比
    // it seems to be reasonable, 'cause end is not included
    if (value >= maxBucketValue_)
      return end - beg - 1;
    else
      return std::lower_bound(beg, end, value) - beg;
  }

  size_t BucketCount() const {
    return bucketValues_.size();
  }

  uint64_t LastValue() const {
    return maxBucketValue_;
  }  

  uint64_t FirstValue() const {
    return minBucketValue_;
  }

  uint64_t BucketLimit(const size_t bucketNumber) const {
    assert(bucketNumber < BucketCount());
    return bucketValues_[bucketNumber];
  }

  // the object has type qualifiers that are not compatible with the member 
  // function "HistogramBucketMapper::PrintBucketValues" -- object type is: 
  // const HistogramBucketMapper

  // You cannot call a non-const member function on a const object or through
  // a const reference or pointer to const.
  // https://stackoverflow.com/questions/24677032/object-has-type-qualifiers-that-are-not-compatible-with-the-member-function
  void PrintBucketValues() const {
    uint64_t b = 0;
    for (int i = 0; i < 10; i++) {
      for (int j = 0; j < 10 && b < BucketCount(); j++) {
        std::cout << bucketValues_[b] << " ";
        b++;
      }
      std::cout << std::endl;
    }
  }

private:
  std::vector<uint64_t> bucketValues_;
  uint64_t maxBucketValue_;
  uint64_t minBucketValue_;
};

namespace {
  const HistogramBucketMapper bucketMapper;
}

struct HistogramStat {
  HistogramStat() : num_buckets_(bucketMapper.BucketCount()) {
    assert(num_buckets_ == sizeof(buckets_) / sizeof(*buckets_));
    Clear();
  }
  ~HistogramStat() {}

  // what's this? Does it mean copy constructor?
  HistogramStat(const HistogramStat&) = delete;
  HistogramStat& operator=(const HistogramStat&) = delete;

  void Clear() {
    // min_ set to max of bucketMapper
    // others set to 0
    min_.store(bucketMapper.LastValue(), std::memory_order_relaxed);
    max_.store(0, std::memory_order_relaxed);
    num_.store(0, std::memory_order_relaxed);
    sum_.store(0, std::memory_order_relaxed);
    sum_squares_.store(0, std::memory_order_relaxed);
    for (unsigned int b = 0; b < num_buckets_; b++) {
      buckets_[b].store(0, std::memory_order_relaxed);
    }
  }
  // 判断是否是0
  bool Empty() const { return num() == 0; }
  void Add(uint64_t value) {
    const size_t index = bucketMapper.IndexForValue(value);
    assert(index < num_buckets_);
    // TODO what's this
    // + 1?
    buckets_[index].store(buckets_[index].load(std::memory_order_relaxed) + 1,
                          std::memory_order_relaxed);
    uint64_t old_min = min();
    if (value < old_min) {
      min_.store(value, std::memory_order_relaxed);
    }
    uint64_t old_max = max();
    if (value > old_max) {
      max_.store(value, std::memory_order_relaxed);
    }

    num_.store(num_.load(std::memory_order_relaxed) + 1,
               std::memory_order_relaxed);
    sum_.store(sum_.load(std::memory_order_relaxed) + value,
               std::memory_order_relaxed);
    sum_squares_.store(
        sum_squares_.load(std::memory_order_relaxed) + value * value,
        std::memory_order_relaxed
    );
  }
  void Merge(const HistogramStat& other) {
    uint64_t old_min = min();
    uint64_t other_min = other.min();
    // 比较与old是否相等 若相等则赋值为other
    // 如果不是，则将变量的值和期待的值交换
    while (other_min < old_min &&
           !min_.compare_exchange_weak(old_min, other_min)) {}
    uint64_t old_max = max();
    uint64_t other_max = other.max();
    while (other_max > old_max &&
           !max_.compare_exchange_weak(old_max, other_max)) {}
    num_.fetch_add(other.num(), std::memory_order_relaxed);
    sum_.fetch_add(other.sum(), std::memory_order_relaxed);
    sum_squares_.fetch_add(other.sum_squares(), std::memory_order_relaxed);
    for (unsigned int b = 0; b < num_buckets_; b++) {
      // 每一个桶都加上另一个的值
      // 2 bucket mapper merge
      buckets_[b].fetch_add(other.bucket_at(b), std::memory_order_relaxed);
    }
  }

  std::atomic_uint_fast64_t min_;
  std::atomic_uint_fast64_t max_;
  std::atomic_uint_fast64_t num_;
  std::atomic_uint_fast64_t sum_;
  std::atomic_uint_fast64_t sum_squares_;
  // this is because of the constructor of bucketMapper
  std::atomic_uint_fast64_t buckets_[109];
  const uint64_t num_buckets_;

  inline uint64_t min() const { return min_.load(std::memory_order_relaxed); }
  inline uint64_t max() const { return max_.load(std::memory_order_relaxed); }
  inline uint64_t num() const { return num_.load(std::memory_order_relaxed); }
  inline uint64_t sum() const { return sum_.load(std::memory_order_relaxed); }
  inline uint64_t sum_squares() const {
    return sum_squares_.load(std::memory_order_relaxed);
  }
  inline uint64_t bucket_at(size_t b) const {
    return buckets_[b].load(std::memory_order_relaxed);
  }

  double Median() const {
    return Percentile(50.0);
  }
  double Percentile(double p) const {
    // bucket中积攒的值的数量的百分比
    double threshold = num() * (p / 100.0);
    uint64_t cumulative_sum = 0;
    for (unsigned int b = 0; b < num_buckets_; b++) {
      uint64_t bucket_value = bucket_at(b);
      cumulative_sum += bucket_value;
      if (cumulative_sum >= threshold) {
        uint64_t left_point = (b == 0) ? 0 : bucketMapper.BucketLimit(b - 1);
        uint64_t right_point = bucketMapper.BucketLimit(b);
        uint64_t left_sum = cumulative_sum - bucket_value;
        uint64_t right_sum = cumulative_sum;
        double pos = 0;
        uint64_t right_left_diff = right_sum - left_sum;
        if (right_left_diff != 0) {
          pos = (threshold - left_sum) / right_left_diff;
        }
        double r = left_point + (right_point - left_point) * pos;
        uint64_t cur_min = min();
        uint64_t cur_max = max();
        if (r < cur_min) r = static_cast<double>(cur_min);
        if (r > cur_max) r = static_cast<double>(cur_max);
        return r;
      }
    }
    return static_cast<double>(max());
  }
  double Average() const {
    uint64_t cur_num = num();
    uint64_t cur_sum = sum();
    if (cur_num == 0) return 0;
    return static_cast<double>(cur_sum) / static_cast<double>(cur_num);
  }
  double StandardDeviation() const;
  void Data(HistogramData* const data) const;
  std::string ToString() const;
};

// 就没懂这个所谓的histogram有什么用
// it seems to be useless
// and only offer a wrapper
class Histogram {
public:
  Histogram() {}
  virtual ~Histogram() {};

  virtual void Clear() = 0;
  virtual bool Empty() const = 0;
  virtual void Add(uint64_t value) = 0;
  virtual void Merge(const Histogram&) = 0;

  // virtual std::string ToString() const = 0;
  virtual const char* Name() const = 0;
  virtual uint64_t min() const = 0;
  virtual uint64_t max() const = 0;
  virtual uint64_t num() const = 0;
  virtual double Median() const = 0;
  virtual double Percentile(double p) const = 0;
  virtual double Average() const = 0;
  // virtual double StandardDeviation() const = 0;
  // virtual void Data(HistogramData* const data) const = 0;
};

class HistogramImpl : public Histogram {
 public:
  HistogramImpl() { Clear(); }

  HistogramImpl(const HistogramImpl&) = delete;
  HistogramImpl& operator=(const HistogramImpl&) = delete;

  virtual void Clear() override {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_.Clear();
  }
  virtual bool Empty() const override {
    return stats_.Empty();
  }
  virtual void Add(uint64_t value) override {
    stats_.Add(value);
  }
  virtual void Merge(const Histogram& other) override {
    if (strcmp(Name(), other.Name()) == 0) {
      Merge(*static_cast<const HistogramImpl*>(&other));
    }
  }
  void Merge(const HistogramImpl& other) {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_.Merge(other.stats_);
  }

  //  undefined reference to `vtable for HistogramImpl'
  // 此报错是由于子类没有实现父类的纯虚函数
  // virtual std::string ToString() const override;
  virtual const char* Name() const override { return "HistogramImpl"; }
  virtual uint64_t min() const override { return stats_.min(); }
  virtual uint64_t max() const override { return stats_.max(); }
  virtual uint64_t num() const override { return stats_.num(); }
  virtual double Median() const override {
    return stats_.Median();
  }
  virtual double Percentile(double p) const override {
    return stats_.Percentile(p);
  }
  virtual double Average() const override {
    return stats_.Average();
  }
  // virtual double StandardDeviation() const override;
  // virtual void Data(HistogramData* const data) const override;

  virtual ~HistogramImpl() {}

  void PrintStatBucket() {
    uint64_t b = 0;
    for (int i = 0; i < 10; i++) {
      for (int j = 0; j < 10 && b < stats_.num_buckets_; j++) {
        std::cout << stats_.bucket_at(b) << " ";
        b++;
      }
      std::cout << std::endl;
    }
  }

 private:
  HistogramStat stats_;
  std::mutex mutex_;
};

void PopulateHistogram(Histogram& histogram,
             uint64_t low, uint64_t high, uint64_t loop = 1) {
  for (; loop > 0; loop--) {
    for (uint64_t i = low; i <= high; i++) {
      histogram.Add(i);
      usleep(1000);
    }
  }
  // make sure each data population at least take some time
  // 10ms
  usleep(10000);
}

int main ()
{
  bucketMapper.PrintBucketValues();
  HistogramImpl histogram;
  PopulateHistogram(histogram, 1, 10, 10);
  histogram.PrintStatBucket();
  std::cout << histogram.Percentile(100.0) << std::endl;
  std::cout << histogram.Percentile(99.0) << std::endl;
  std::cout << histogram.Percentile(95.0) << std::endl;
  std::cout << histogram.Median() << std::endl;
  std::cout << histogram.Average() << std::endl;
}
