#include <gtest/gtest.h>
#include "memory/UniquePtr.h"
#include <string>

// --- Helper for Lifecycle Tracking ---
struct Tracker {
    static int constructed_count;
    static int destructed_count;
    int value;

    Tracker(int v) : value(v) { constructed_count++; }
    ~Tracker() { destructed_count++; }

    static void reset() {
        constructed_count = 0;
        destructed_count = 0;
    }
};

int Tracker::constructed_count = 0;
int Tracker::destructed_count = 0;

class UniquePtrTest : public ::testing::Test {
protected:
    void SetUp() override {
        Tracker::reset();
    }
};

// 1. Basic Construction & Destruction
TEST_F(UniquePtrTest, DefaultConstructor) {
    My::UniquePtr<int> p;
    EXPECT_EQ(p.get(), nullptr);
    EXPECT_FALSE(p);
}

TEST_F(UniquePtrTest, RawPointerConstructor) {
    {
        My::UniquePtr<Tracker> p(new Tracker(10));
        EXPECT_EQ(Tracker::constructed_count, 1);
        EXPECT_EQ(p->value, 10);
        EXPECT_TRUE(p);
    } // p goes out of scope here
    EXPECT_EQ(Tracker::destructed_count, 1);
}

// 2. Move Semantics
TEST_F(UniquePtrTest, MoveConstructor) {
    My::UniquePtr<Tracker> p1(new Tracker(100));
    My::UniquePtr<Tracker> p2(std::move(p1));

    EXPECT_EQ(p1.get(), nullptr); // p1 should be empty
    EXPECT_NE(p2.get(), nullptr); // p2 should own it
    EXPECT_EQ(p2->value, 100);
    EXPECT_EQ(Tracker::constructed_count, 1);
    EXPECT_EQ(Tracker::destructed_count, 0);
}

TEST_F(UniquePtrTest, MoveAssignment) {
    My::UniquePtr<Tracker> p1(new Tracker(100));
    My::UniquePtr<Tracker> p2(new Tracker(200));

    p2 = std::move(p1);
    // p2's old object (200) should be destroyed.
    // p2 should now own (100).
    // p1 should be empty.

    EXPECT_EQ(p1.get(), nullptr);
    EXPECT_EQ(p2->value, 100);
    EXPECT_EQ(Tracker::constructed_count, 2);
    EXPECT_EQ(Tracker::destructed_count, 1); // Only the "200" tracker destroyed so far
}

TEST_F(UniquePtrTest, MoveAssignmentSelf) {
    My::UniquePtr<Tracker> p1(new Tracker(100));
    p1 = std::move(p1); // Self-move

    EXPECT_NE(p1.get(), nullptr);
    EXPECT_EQ(p1->value, 100);
    EXPECT_EQ(Tracker::destructed_count, 0); // Should not destroy itself
}

// 3. Modifiers
TEST_F(UniquePtrTest, Release) {
    My::UniquePtr<Tracker> p(new Tracker(50));
    Tracker* raw = p.release();

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(Tracker::destructed_count, 0); // Release implies "I take responsibility"

    delete raw;
    EXPECT_EQ(Tracker::destructed_count, 1);
}

TEST_F(UniquePtrTest, Reset) {
    My::UniquePtr<Tracker> p(new Tracker(1));
    p.reset(new Tracker(2)); // Should delete 1, take 2

    EXPECT_EQ(Tracker::destructed_count, 1);
    EXPECT_EQ(p->value, 2);

    p.reset(); // Should delete 2
    EXPECT_EQ(Tracker::destructed_count, 2);
    EXPECT_EQ(p.get(), nullptr);
}

TEST_F(UniquePtrTest, DereferenceOperators) {
    My::UniquePtr<std::string> p(new std::string("hello"));
    EXPECT_EQ(*p, "hello");
    EXPECT_EQ(p->length(), 5);
}

// 4. make_unique
TEST_F(UniquePtrTest, MakeUniqueArgs) {
    auto p = My::make_unique<Tracker>(999);
    EXPECT_EQ(Tracker::constructed_count, 1);
    EXPECT_EQ(p->value, 999);
}
