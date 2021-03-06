#pragma once
#include "stdafx.h"

// from http://howardhinnant.github.io/date_algorithms.html

namespace date_util {
   // Returns number of days since civil 1970-01-01.  Negative key indicate
   //    days prior to 1970-01-01.
   // Preconditions:  y-m-d represents a date in the civil (Gregorian) calendar
   //                 m is in [1, 12]
   //                 d is in [1, last_day_of_month(y, m)]
   //                 y is "approximately" in
   //                   [numeric_limits<Int>::min()/366, numeric_limits<Int>::max()/366]
   //                 Exact range of validity is:
   //                 [civil_from_days(numeric_limits<Int>::min()),
   //                  civil_from_days(numeric_limits<Int>::max()-719468)]
   template <class Int>
   constexpr
      Int
      days_from_civil(Int y, unsigned m, unsigned d) noexcept
   {
      static_assert(std::numeric_limits<unsigned>::digits >= 18,
         "This algorithm has not been ported to a 16 bit unsigned integer");
      static_assert(std::numeric_limits<Int>::digits >= 20,
         "This algorithm has not been ported to a 16 bit signed integer");
      y -= m <= 2;
      const Int era = (y >= 0 ? y : y - 399) / 400;
      const unsigned yoe = static_cast<unsigned>(y - era * 400);      // [0, 399]
      const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;  // [0, 365]
      const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;         // [0, 146096]
      return era * 146097 + static_cast<Int>(doe) - 719468;
   }

   // Returns year/month/day triple in civil calendar
   // Preconditions:  z is number of days since 1970-01-01 and is in the range:
   //                   [numeric_limits<Int>::min(), numeric_limits<Int>::max()-719468].
   template <class Int>
   constexpr
      std::tuple<Int, unsigned, unsigned>
      civil_from_days(Int z) noexcept
   {
      static_assert(std::numeric_limits<unsigned>::digits >= 18,
         "This algorithm has not been ported to a 16 bit unsigned integer");
      static_assert(std::numeric_limits<Int>::digits >= 20,
         "This algorithm has not been ported to a 16 bit signed integer");
      z += 719468;
      const Int era = (z >= 0 ? z : z - 146096) / 146097;
      const unsigned doe = static_cast<unsigned>(z - era * 146097);          // [0, 146096]
      const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;  // [0, 399]
      const Int y = static_cast<Int>(yoe) + era * 400;
      const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);                // [0, 365]
      const unsigned mp = (5 * doy + 2) / 153;                                   // [0, 11]
      const unsigned d = doy - (153 * mp + 2) / 5 + 1;                             // [1, 31]
      const unsigned m = mp + (mp < 10 ? 3 : -9);                            // [1, 12]
      return std::tuple<Int, unsigned, unsigned>(y + (m <= 2), m, d);
   }

   // Returns: true if y is a leap year in the civil calendar, else false
   template <class Int>
   constexpr
      bool
      is_leap(Int y) noexcept
   {
      return  y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
   }

   // Preconditions: m is in [1, 12]
   // Returns: The number of days in the month m of common year
   // The result is always in the range [28, 31].
   //constexpr
      unsigned
      last_day_of_month_common_year(unsigned m) noexcept
   {
      constexpr unsigned char a[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
      return a[m - 1];
   }

   // Preconditions: m is in [1, 12]
   // Returns: The number of days in the month m of leap year
   // The result is always in the range [29, 31].
   //constexpr
      unsigned
      last_day_of_month_leap_year(unsigned m) noexcept
   {
      constexpr unsigned char a[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
      return a[m - 1];
   }

   // Preconditions: m is in [1, 12]
   // Returns: The number of days in the month m of year y
   // The result is always in the range [28, 31].
   template <class Int>
   constexpr
      unsigned
      last_day_of_month(Int y, unsigned m) noexcept
   {
      return m != 2 || !is_leap(y) ? last_day_of_month_common_year(m) : 29u;
   }

   // Returns day of week in civil calendar [0, 6] -> [Sun, Sat]
   // Preconditions:  z is number of days since 1970-01-01 and is in the range:
   //                   [numeric_limits<Int>::min(), numeric_limits<Int>::max()-4].
   template <class Int>
   constexpr
      unsigned
      weekday_from_days(Int z) noexcept
   {
      return static_cast<unsigned>(z >= -4 ? (z + 4) % 7 : (z + 5) % 7 + 6);
   }

   // Preconditions: x <= 6 && y <= 6
   // Returns: The number of days from the weekday y to the weekday x.
   // The result is always in the range [0, 6].
   //constexpr
      unsigned
      weekday_difference(unsigned x, unsigned y) noexcept
   {
      x -= y;
      return x <= 6 ? x : x + 7;
   }

   // Preconditions: wd <= 6
   // Returns: The weekday following wd
   // The result is always in the range [0, 6].
   constexpr
      unsigned
      next_weekday(unsigned wd) noexcept
   {
      return wd < 6 ? wd + 1 : 0;
   }

   // Preconditions: wd <= 6
   // Returns: The weekday prior to wd
   // The result is always in the range [0, 6].
   inline
      constexpr
      unsigned
      prev_weekday(unsigned wd) noexcept
   {
      return wd > 0 ? wd - 1 : 6;
   }

   /////////////////////////////////////////////////////////////////////////////////

   static time_t mkgmtime(const struct tm *ptm) {
      static const int SecondsPerMinute = 60;
      static const int SecondsPerHour = 3600;
      static const int SecondsPerDay = 86400;
      static const int DaysOfMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

      time_t secs = 0;
      // tm_year is years since 1900
      int year = ptm->tm_year + 1900;
      for (int y = 1970; y < year; ++y) {
         secs += (is_leap(y) ? 366 : 365) * SecondsPerDay;
      }
      // tm_mon is month from 0..11
      for (int m = 0; m < ptm->tm_mon; ++m) {
         secs += DaysOfMonth[m] * SecondsPerDay;
         if (m == 1 && is_leap(year)) secs += SecondsPerDay;
      }
      secs += (ptm->tm_mday - 1) * SecondsPerDay;
      secs += ptm->tm_hour       * SecondsPerHour;
      secs += ptm->tm_min        * SecondsPerMinute;
      secs += ptm->tm_sec;
      return secs;
   }

   static time_t mkgmtime(int year, int mon, int mday, int hour = 0, int min = 0, int sec = 0) {
      std::tm time = { 0 };

      time.tm_sec = sec;
      time.tm_min = min;
      time.tm_hour = hour;

      time.tm_mday = mday;
      time.tm_mon = mon - 1;
      time.tm_year = year - 1900;

      return mkgmtime(&time);
   }

} // namespace date_util
