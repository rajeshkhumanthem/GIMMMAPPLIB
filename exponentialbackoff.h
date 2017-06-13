#ifndef EXPONENTIALBACKOFF_H
#define EXPONENTIALBACKOFF_H


#include <random>
#include <cmath>

/*!
 * \brief The ExponentialBackoff class
 */
class ExponentialBackoff
{
      std::random_device __seeder;
      std::mt19937 __engine;
      std::uniform_int_distribution<int> __distribution;
      int __retry;
      int __maxRetry;
    public:
      ExponentialBackoff(int start_val = 0, int max_retry = -1);
      ExponentialBackoff(const ExponentialBackoff& rhs);
      const ExponentialBackoff& operator =(const ExponentialBackoff& rhs);
      int getRetry() const { return __retry;}
      int getMaxRetry() const { return __maxRetry;}

      int next();
      void resetRetry() { __retry = 0;}
};


/*!
 * \brief ExponentialBackoff::ExponentialBackoff
 * \param start_val
 * \param max_retry
 */
inline ExponentialBackoff::ExponentialBackoff(int start_val, int max_retry)
      :__engine(__seeder()),
        __distribution(100, 1000),
        __retry(start_val),
        __maxRetry(max_retry)
{
        if ( max_retry > -1)
            __maxRetry = max_retry + start_val;

}


inline ExponentialBackoff::ExponentialBackoff(const ExponentialBackoff &rhs)
{
    __engine = rhs.__engine;
    __distribution = rhs.__distribution;
    __retry = rhs.__retry;
    __maxRetry = rhs.__maxRetry;
}


inline const ExponentialBackoff& ExponentialBackoff::operator =(const ExponentialBackoff& rhs)
{
    if ( &rhs != this)
    {
        __engine = rhs.__engine;
        __distribution = rhs.__distribution;
        __retry = rhs.__retry;
        __maxRetry = rhs.__maxRetry;
    }
    return *this;
}

/*!
 * \brief ExponentialBackoff::next
 * \return
 */
inline int ExponentialBackoff::next()
{
    if ((__maxRetry != -1) &&
            (__retry > __maxRetry))
      return -1;

    int random_delta = __distribution(__engine);
    int r = 0.5 * (pow(2, __retry) - 1);

    int next = r * 1000 + random_delta;
    __retry++;
    return next;
}
#endif // EXPONENTIALBACKOFF_H
