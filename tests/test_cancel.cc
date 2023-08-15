#include <gtest/gtest.h>
#include <signal.h>
#include <noshell/noshell.hpp>

namespace {
namespace NS = noshell;
using namespace NS::literal;

TEST(Cancel, Cancel) {

  auto p = "sleep"_C("10000");
  auto e = p.run();

  sleep(1);
  e.cancel();
  e.wait();

  EXPECT_FALSE(e[0].setup_error());
  EXPECT_TRUE(e[0].data.status.signaled());
  EXPECT_EQ(15, e[0].data.status.term_sig());

} // Cancel.Cancel
} // empty namespace
