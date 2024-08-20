#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctime>

namespace unitauto::test {

     static double divide(int a, int b) {
          return a/b;
     }

     static int contains(long arr[], long l) {
          if (arr == nullptr) {
               return -1;
          }

          for (int i = 0; i < sizeof arr; ++i) {
               if (arr[i] == l) {
                    return i;
               }
          }

          return -1;
     }

     static int index(std::string arr[], std::string s) {
          if (arr == nullptr || arr->length() <= 0) {
               return -1;
          }

          for (int i = 0; i < arr->length(); ++i) {
               if (arr[i] == s) {
                    return i;
               }
          }

          return -1;
     }

     class TestUtil {
          static double divide(double a, double b) {
               return a/b;
          }
     };

}