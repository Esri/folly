/*
 * Copyright 2017-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <folly/io/async/SSLOptions.h>
#include <folly/io/async/SSLContext.h>
#include <folly/portability/GTest.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

using namespace std;
using namespace testing;

namespace folly {

class SSLOptionsTest : public testing::Test {};

TEST_F(SSLOptionsTest, TestSetCommonCipherList) {
  SSLContext ctx;
  ssl::setCipherSuites<ssl::SSLCommonOptions>(ctx);

  int i = 0;
  ssl::SSLUniquePtr ssl(ctx.createSSL());
  for (auto& cipher : ssl::SSLCommonOptions::ciphers()) {
    ASSERT_STREQ(cipher, SSL_get_cipher_list(ssl.get(), i++));
  }
  ASSERT_EQ(nullptr, SSL_get_cipher_list(ssl.get(), i));
}

TEST_F(SSLOptionsTest, TestSetCipherListWithVector) {
  SSLContext ctx;
  auto ciphers = ssl::SSLCommonOptions::ciphers();
  ssl::setCipherSuites(ctx, ciphers);

  int i = 0;
  ssl::SSLUniquePtr ssl(ctx.createSSL());
  for (auto& cipher : ssl::SSLCommonOptions::ciphers()) {
    ASSERT_STREQ(cipher, SSL_get_cipher_list(ssl.get(), i++));
  }
  ASSERT_EQ(nullptr, SSL_get_cipher_list(ssl.get(), i));
}

} // namespace folly
