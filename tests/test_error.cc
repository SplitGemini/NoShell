#include <gtest/gtest.h>
#include <signal.h>
#include <noshell/noshell.hpp>
#include "libtest_misc.hpp"


namespace {
namespace NS = noshell;
using namespace NS::literal;

TEST(Error, ReadFile) {
  check_fixed_fds check_fds;

  NS::Exit e = "cat"_C() < "doesntexists";

  EXPECT_TRUE(e[0].setup_error());
  const std::string expect = "Failed to open the file 'doesntexists' for reading";
  EXPECT_EQ(expect, e[0].message);
  EXPECT_EQ(ENOENT, e[0].data.err.value);
} // Error.ReadFile

TEST(Error, WriteFile) {
  check_fixed_fds check_fds;

  #define nadirw "WriteFileNotAllowed"
  not_allowed_dir dir(nadirw);
  {
    NS::Exit e = "cat"_C() > nadirw "/noallowed";
    ASSERT_TRUE(e[0].setup_error());
    const std::string expect = "Failed to open the file '" nadirw "/noallowed' for writing";
    EXPECT_EQ(expect, e[0].message);
    EXPECT_EQ(EACCES, e[0].data.err.value);
  }

  {
    NS::Exit e = "cat"_C() > nadirw "/doesntexists/stupid";
    ASSERT_TRUE(e[0].setup_error());
    const std::string expect = "Failed to open the file '" nadirw "/doesntexists/stupid' for writing";
    EXPECT_EQ(expect, e[0].message);
    EXPECT_EQ(ENOENT, e[0].data.err.value);
  }
} // Error.ReadFile

TEST(Error, BadCmd) {
  check_fixed_fds check_fds;

  NS::Exit e = "stupidcmd"_C();
  ASSERT_TRUE(e[0].setup_error());
  EXPECT_EQ("Child process setup error", e[0].message);
  // EXPECT_EQ(ENOENT, e[0].data.err.value);
  EXPECT_NE(0, e[0].data.err.value);
} // Error.BadCmd

TEST(Error, Failures) {
  check_fixed_fds check_fds;
  #define nadirf "FailuresNotAllowed"
  not_allowed_dir dir(nadirf);

  NS::Exit e = ("notexists"_C() | ("cat"_C("--badoption") > 2_R("/dev/null")) | "cat"_C()) > nadirf "/noallowed";
  EXPECT_FALSE(e.success());

  {
    ssize_t i = 0;
    for(const auto& it : e.failures()) {
      EXPECT_EQ(i, e.id(it));
      if(i == 0) {
        EXPECT_TRUE(it.setup_error());
        //      EXPECT_EQ(ENOENT, it.err().value);
        EXPECT_NE(0, it.err().value);
      } else if(i == 1) {
        EXPECT_FALSE(it.setup_error());
        EXPECT_TRUE(it.have_status());
        EXPECT_NE(0, it.status().exit_status());
      } else if(i == 2) {
        EXPECT_TRUE(it.setup_error());
        EXPECT_EQ(EACCES, it.err().value);
      }
      ++i;
    }
  }

  {
    ssize_t i = 0;
    for(const auto& it : e) {
      EXPECT_EQ(i, e.id(it));
      ++i;
    }
    EXPECT_EQ((ssize_t)3, i);
  }
} // Error.Failures

TEST(Error, SigPipe) {
  // Make sure SIGPIPE is delivered to cat
  sigset_t signals;
  ASSERT_EQ(0, sigemptyset(&signals));
  ASSERT_EQ(0, sigaddset(&signals, SIGPIPE));
  ASSERT_EQ(0, sigprocmask(SIG_UNBLOCK, &signals, nullptr));
  NS::Exit e = ("cat"_C("/dev/zero") | "head"_C("-c", 1)) > "/dev/null";
  std::cerr << e << std::endl;
  EXPECT_FALSE(e.success());
  EXPECT_TRUE(e.success(true));
  EXPECT_FALSE(e[0].success());
  EXPECT_TRUE(e[0].success(true));
  EXPECT_TRUE(e[1].success());
} // Error.SigPipe

} // empty namespace
