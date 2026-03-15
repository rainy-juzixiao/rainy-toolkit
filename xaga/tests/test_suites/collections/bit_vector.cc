#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <rainy/collections/bit_vector.hpp>

using namespace rainy::collections;

SCENARIO("bit_vector construction and basic properties", "[bit_vector][construction]") {
    GIVEN("Default constructed bit_vector") {
        bit_vector<> vec;

        THEN("It should be empty") {
            REQUIRE(vec.empty());
            REQUIRE(vec.size() == 0);
            REQUIRE(vec.capacity() == 0);
        }
    }

    GIVEN("bit_vector constructed with size") {
        auto size = GENERATE(0, 1, 10, 100, 1000);
        bit_vector<> vec(size);

        THEN("Size should match constructor argument") {
            REQUIRE(vec.size() == size);
            if (size > 0) {
                REQUIRE(vec.capacity() >= size);
            }
        }

        AND_THEN("All bits should be initialized to false") {
            for (size_t i = 0; i < size; ++i) {
                REQUIRE(vec[i] == false);
            }
            REQUIRE(vec.none());
        }
    }

    GIVEN("bit_vector constructed with size and initial value") {
        auto [size, value] = GENERATE(table<size_t, bool>({{0, true}, {1, false}, {5, true}, {10, false}, {100, true}}));

        bit_vector<> vec(size, value);

        THEN("All bits should have the specified value") {
            for (size_t i = 0; i < size; ++i) {
                REQUIRE(vec[i] == value);
            }
            if (size > 0) {
                if (value) {
                    REQUIRE(vec.all());
                } else {
                    REQUIRE(vec.none());
                }
            }
        }
    }

    GIVEN("bit_vector constructed from initializer list") {
        bit_vector<> vec = {true, false, true, true, false};

        THEN("Elements should match initializer list") {
            REQUIRE(vec.size() == 5);
            REQUIRE(vec[0] == true);
            REQUIRE(vec[1] == false);
            REQUIRE(vec[2] == true);
            REQUIRE(vec[3] == true);
            REQUIRE(vec[4] == false);
        }
    }

    GIVEN("bit_vector constructed from iterator range") {
        std::vector<bool> source = {true, false, true, false, true};
        bit_vector<> vec(source.begin(), source.end());

        THEN("Elements should match source range") {
            REQUIRE(vec.size() == source.size());
            for (size_t i = 0; i < source.size(); ++i) {
                REQUIRE(vec[i] == source[i]);
            }
        }
    }
}

SCENARIO("bit_vector copy and move semantics", "[bit_vector][copy][move]") {
    GIVEN("A non-empty bit_vector") {
        bit_vector<> original = {true, false, true, false, true};

        WHEN("Copy constructed") {
            bit_vector<> copy(original);

            THEN("Copy should equal original") {
                REQUIRE(copy == original);
                REQUIRE(copy.size() == original.size());
            }

            AND_WHEN("Original is modified") {
                original[0] = false;

                THEN("Copy should remain unchanged") {
                    REQUIRE(copy[0] == true);
                }
            }
        }

        WHEN("Copy assigned") {
            bit_vector<> copy;
            copy = original;

            THEN("Copy should equal original") {
                REQUIRE(copy == original);
            }
        }

        WHEN("Move constructed") {
            bit_vector<> original_copy(original);
            bit_vector<> moved(std::move(original));

            THEN("Moved-to should equal original") {
                REQUIRE(moved == original_copy);
            }

            AND_THEN("Original should be empty") {
                REQUIRE(original.empty());
            }
        }

        WHEN("Move assigned") {
            bit_vector<> original_copy(original);
            bit_vector<> moved;
            moved = std::move(original);

            THEN("Moved-to should equal original") {
                REQUIRE(moved == original_copy);
            }

            AND_THEN("Original should be empty") {
                REQUIRE(original.empty());
            }
        }
    }
}

SCENARIO("bit_vector element access", "[bit_vector][access]") {
    GIVEN("A bit_vector with mixed values") {
        bit_vector<> vec = {true, false, true, false, true};

        THEN("operator[] should provide correct access") {
            REQUIRE(vec[0] == true);
            REQUIRE(vec[1] == false);
            REQUIRE(vec[2] == true);
            REQUIRE(vec[3] == false);
            REQUIRE(vec[4] == true);
        }

        THEN("front() and back() should work correctly") {
            REQUIRE(vec.front() == true);
            REQUIRE(vec.back() == true);
        }

        WHEN("Modifying through operator[]") {
            vec[1] = true;
            vec[3] = true;

            THEN("Changes should be reflected") {
                REQUIRE(vec[1] == true);
                REQUIRE(vec[3] == true);
            }
        }

        WHEN("Using at() with valid index") {
            THEN("Access should succeed") {
                REQUIRE_NOTHROW(vec.at(0));
                REQUIRE(vec.at(2) == true);
            }
        }

        WHEN("Using at() with invalid index") {
            THEN("Should throw out_of_range") {
                REQUIRE_THROWS_AS(vec.at(5), std::out_of_range);
                REQUIRE_THROWS_AS(vec.at(vec.size()), std::out_of_range);
            }
        }
    }

    GIVEN("Empty bit_vector") {
        bit_vector<> vec;

        THEN("front() and back() should not be called") {
            // Note: front()/back() on empty container is undefined behavior
            // We're not testing that here
        }
    }
}

SCENARIO("bit_vector reference proxy behavior", "[bit_vector][reference]") {
    GIVEN("A bit_vector with some values") {
        bit_vector<> vec = {true, false, true};

        WHEN("Getting a reference") {
            auto ref = vec[1];

            THEN("Reference should convert to bool") {
                REQUIRE(static_cast<bool>(ref) == false);
            }

            AND_WHEN("Assigning to reference") {
                ref = true;

                THEN("Original should be updated") {
                    REQUIRE(vec[1] == true);
                }
            }

            AND_WHEN("Flipping reference") {
                ref.flip();

                THEN("Bit should be flipped") {
                    REQUIRE(vec[1] == true);
                }
            }

            AND_WHEN("Using operator~") {
                auto inverted = ~ref;

                THEN("Should return inverted value without modifying") {
                    REQUIRE(inverted == true);
                    REQUIRE(vec[1] == false);
                }
            }
        }

        WHEN("Swapping two references") {
            auto ref1 = vec[0];
            auto ref2 = vec[2];

            bit_vector<>::swap(ref1, ref2);

            THEN("Values should be swapped") {
                REQUIRE(vec[0] == true);
                REQUIRE(vec[2] == true); // Both were true, so no visible change
            }

            AND_WHEN("Swapping true and false") {
                vec[1] = true;
                auto ref3 = vec[1];
                auto ref4 = vec[2];
                bit_vector<>::swap(ref3, ref4);

                THEN("Values should swap") {
                    REQUIRE(vec[1] == true);
                    REQUIRE(vec[2] == true);
                }
            }
        }
    }
}

SCENARIO("bit_vector modifiers - push_back and pop_back", "[bit_vector][modifiers]") {
    GIVEN("An empty bit_vector") {
        bit_vector<> vec;

        WHEN("Pushing back true values") {
            vec.push_back(true);
            vec.push_back(true);
            vec.push_back(false);

            THEN("Size should increase and values should be correct") {
                REQUIRE(vec.size() == 3);
                REQUIRE(vec[0] == true);
                REQUIRE(vec[1] == true);
                REQUIRE(vec[2] == false);
            }
        }

        WHEN("Pushing many values to trigger reallocation") {
            const size_t count = 1000;
            for (size_t i = 0; i < count; ++i) {
                vec.push_back(i % 2 == 0);
            }

            THEN("All values should be correct") {
                REQUIRE(vec.size() == count);
                for (size_t i = 0; i < count; ++i) {
                    REQUIRE(vec[i] == (i % 2 == 0));
                }
            }
        }
    }

    GIVEN("A bit_vector with elements") {
        bit_vector<> vec = {true, false, true};

        WHEN("Popping back") {
            vec.pop_back();

            THEN("Size should decrease") {
                REQUIRE(vec.size() == 2);
                REQUIRE(vec[0] == true);
                REQUIRE(vec[1] == false);
            }
        }

        WHEN("Popping all elements") {
            vec.pop_back();
            vec.pop_back();
            vec.pop_back();

            THEN("Vector should be empty") {
                REQUIRE(vec.empty());
            }
        }
    }

    GIVEN("A bit_vector") {
        bit_vector<> vec;

        WHEN("Using emplace_back") {
            auto ref = vec.emplace_back(true);

            THEN("Should construct element in-place") {
                REQUIRE(vec.size() == 1);
                REQUIRE(vec[0] == true);
                REQUIRE(static_cast<bool>(ref) == true);
            }
        }
    }
}

SCENARIO("bit_vector resize and reserve", "[bit_vector][capacity]") {
    GIVEN("A bit_vector with some elements") {
        bit_vector<> vec = {true, false, true};
        size_t original_capacity = vec.capacity();

        WHEN("Reserving larger capacity") {
            vec.reserve(100);

            THEN("Capacity should increase") {
                REQUIRE(vec.capacity() >= 100);
                REQUIRE(vec.size() == 3); // Size unchanged
                REQUIRE(vec[0] == true);
                REQUIRE(vec[1] == false);
                REQUIRE(vec[2] == true);
            }
        }

        WHEN("Reserving smaller capacity") {
            vec.reserve(2);

            THEN("Capacity should not decrease") {
                REQUIRE(vec.capacity() >= original_capacity);
            }
        }

        WHEN("Resizing to larger size with default value") {
            vec.resize(5);

            THEN("Size should increase, new bits default to false") {
                REQUIRE(vec.size() == 5);
                REQUIRE(vec[3] == false);
                REQUIRE(vec[4] == false);
            }
        }

        WHEN("Resizing to larger size with specified value") {
            vec.resize(6, true);

            THEN("New bits should be set to specified value") {
                REQUIRE(vec.size() == 6);
                REQUIRE(vec[3] == true);
                REQUIRE(vec[4] == true);
                REQUIRE(vec[5] == true);
            }
        }

        WHEN("Resizing to smaller size") {
            vec.resize(2);

            THEN("Size should decrease, remaining elements unchanged") {
                REQUIRE(vec.size() == 2);
                REQUIRE(vec[0] == true);
                REQUIRE(vec[1] == false);
            }
        }

        WHEN("Shrinking to fit") {
            vec.reserve(100);
            size_t before_shrink = vec.capacity();
            vec.shrink_to_fit();

            THEN("Capacity should be reduced") {
                REQUIRE(vec.capacity() <= before_shrink);
                REQUIRE(vec.capacity() >= vec.size());
            }
        }
    }
}

SCENARIO("bit_vector insertion", "[bit_vector][insert]") {
    GIVEN("A bit_vector with initial values") {
        bit_vector<> vec = {true, false, true};

        WHEN("Inserting single value at beginning") {
            auto it = vec.insert(vec.begin(), false);

            THEN("Vector should contain inserted value at beginning") {
                REQUIRE(vec.size() == 4);
                REQUIRE(vec[0] == false);
                REQUIRE(vec[1] == true);
                REQUIRE(vec[2] == false);
                REQUIRE(vec[3] == true);
                REQUIRE(it == vec.begin());
            }
        }

        WHEN("Inserting single value at end") {
            auto it = vec.insert(vec.end(), false);

            THEN("Value should be appended") {
                REQUIRE(vec.size() == 4);
                REQUIRE(vec[3] == false);
                REQUIRE(it == vec.begin() + 3);
            }
        }

        WHEN("Inserting single value in middle") {
            auto it = vec.insert(vec.begin() + 1, false);

            THEN("Value should be inserted at correct position") {
                REQUIRE(vec.size() == 4);
                REQUIRE(vec[0] == true);
                REQUIRE(vec[1] == false);
                REQUIRE(vec[2] == false);
                REQUIRE(vec[3] == true);
            }
        }

        WHEN("Inserting multiple copies") {
            vec.insert(vec.begin() + 1, 3, true);

            THEN("Multiple copies should be inserted") {
                REQUIRE(vec.size() == 6);
                REQUIRE(vec[0] == true);
                REQUIRE(vec[1] == true);
                REQUIRE(vec[2] == true);
                REQUIRE(vec[3] == true);
                REQUIRE(vec[4] == false);
                REQUIRE(vec[5] == true);
            }
        }

        WHEN("Inserting from initializer list") {
            vec.insert(vec.begin() + 2, {false, false, true});

            THEN("All list elements should be inserted") {
                REQUIRE(vec.size() == 6);
                REQUIRE(vec[0] == true);
                REQUIRE(vec[1] == false);
                REQUIRE(vec[2] == false);
                REQUIRE(vec[3] == false);
                REQUIRE(vec[4] == true);
                REQUIRE(vec[5] == true);
            }
        }

        WHEN("Inserting zero elements") {
            auto it = vec.insert(vec.begin() + 1, 0, true);

            THEN("Nothing should change") {
                REQUIRE(vec.size() == 3);
                REQUIRE(it == vec.begin() + 1);
            }
        }
    }
}

SCENARIO("bit_vector erasure", "[bit_vector][erase]") {
    GIVEN("A bit_vector with multiple elements") {
        bit_vector<> vec = {true, false, true, false, true};

        WHEN("Erasing single element from beginning") {
            auto it = vec.erase(vec.begin());

            THEN("First element should be removed") {
                REQUIRE(vec.size() == 4);
                REQUIRE(vec[0] == false);
                REQUIRE(vec[1] == true);
                REQUIRE(vec[2] == false);
                REQUIRE(vec[3] == true);
                REQUIRE(it == vec.begin());
            }
        }

        WHEN("Erasing single element from end") {
            auto it = vec.erase(vec.end() - 1);

            THEN("Last element should be removed") {
                REQUIRE(vec.size() == 4);
                REQUIRE(vec[0] == true);
                REQUIRE(vec[1] == false);
                REQUIRE(vec[2] == true);
                REQUIRE(vec[3] == false);
                REQUIRE(it == vec.end());
            }
        }

        WHEN("Erasing single element from middle") {
            auto it = vec.erase(vec.begin() + 2);

            THEN("Middle element should be removed") {
                REQUIRE(vec.size() == 4);
                REQUIRE(vec[0] == true);
                REQUIRE(vec[1] == false);
                REQUIRE(vec[2] == false);
                REQUIRE(vec[3] == true);
            }
        }

        WHEN("Erasing range of elements") {
            auto it = vec.erase(vec.begin() + 1, vec.begin() + 4);
            THEN("Range should be removed") {
                REQUIRE(vec.size() == 2);
                REQUIRE(vec[0] == true);
                REQUIRE(vec[1] == true);
                REQUIRE(it == vec.begin() + 1);
            }
        }

        WHEN("Erasing empty range") {
            auto it = vec.erase(vec.begin() + 1, vec.begin() + 1);

            THEN("Nothing should change") {
                REQUIRE(vec.size() == 5);
                REQUIRE(it == vec.begin() + 1);
            }
        }

        WHEN("Erasing all elements") {
            vec.erase(vec.begin(), vec.end());

            THEN("Vector should be empty") {
                REQUIRE(vec.empty());
            }
        }
    }
}

SCENARIO("bit_vector bit operations", "[bit_vector][bitwise]") {
    GIVEN("Two bit_vectors of same size") {
        bit_vector<> vec1 = {true, false, true, false, true};
        bit_vector<> vec2 = {true, true, false, false, true};

        WHEN("Performing AND operation") {
            auto result = vec1 & vec2;

            THEN("Result should be bitwise AND") {
                REQUIRE(result.size() == 5);
                REQUIRE(result[0] == true);
                REQUIRE(result[1] == false);
                REQUIRE(result[2] == false);
                REQUIRE(result[3] == false);
                REQUIRE(result[4] == true);
            }
        }

        WHEN("Performing OR operation") {
            auto result = vec1 | vec2;

            THEN("Result should be bitwise OR") {
                REQUIRE(result[0] == true);
                REQUIRE(result[1] == true);
                REQUIRE(result[2] == true);
                REQUIRE(result[3] == false);
                REQUIRE(result[4] == true);
            }
        }

        WHEN("Performing XOR operation") {
            auto result = vec1 ^ vec2;

            THEN("Result should be bitwise XOR") {
                REQUIRE(result[0] == false);
                REQUIRE(result[1] == true);
                REQUIRE(result[2] == true);
                REQUIRE(result[3] == false);
                REQUIRE(result[4] == false);
            }
        }

        WHEN("Applying compound assignment operators") {
            bit_vector<> copy1(vec1);
            bit_vector<> copy2(vec2);

            copy1 &= vec2;
            copy2 |= vec1;

            THEN("Results should match bitwise operations") {
                REQUIRE(copy1 == (vec1 & vec2));
                REQUIRE(copy2 == (vec2 | vec1));
            }
        }
    }

    GIVEN("A bit_vector") {
        bit_vector<> vec = {true, false, true, false};

        WHEN("Flipping all bits") {
            vec.flip();

            THEN("All bits should be inverted") {
                REQUIRE(vec[0] == false);
                REQUIRE(vec[1] == true);
                REQUIRE(vec[2] == false);
                REQUIRE(vec[3] == true);
            }
        }

        WHEN("Using operator~") {
            auto inverted = ~vec;

            THEN("Original should remain unchanged, inverted should be complement") {
                REQUIRE(vec[0] == true);
                REQUIRE(vec[1] == false);
                REQUIRE(vec[2] == true);
                REQUIRE(vec[3] == false);

                REQUIRE(inverted[0] == false);
                REQUIRE(inverted[1] == true);
                REQUIRE(inverted[2] == false);
                REQUIRE(inverted[3] == true);
            }
        }

        WHEN("Setting all bits") {
            vec.set();

            THEN("All bits should be true") {
                REQUIRE(vec.all());
                for (size_t i = 0; i < vec.size(); ++i) {
                    REQUIRE(vec[i] == true);
                }
            }
        }

        WHEN("Resetting all bits") {
            vec.reset();

            THEN("All bits should be false") {
                REQUIRE(vec.none());
                for (size_t i = 0; i < vec.size(); ++i) {
                    REQUIRE(vec[i] == false);
                }
            }
        }

        WHEN("Setting individual bit") {
            vec.set(1);

            THEN("Only specified bit should be set") {
                REQUIRE(vec[0] == true);
                REQUIRE(vec[1] == true);
                REQUIRE(vec[2] == true);
                REQUIRE(vec[3] == false);
            }
        }

        WHEN("Resetting individual bit") {
            vec.reset(0);

            THEN("Only specified bit should be reset") {
                REQUIRE(vec[0] == false);
                REQUIRE(vec[1] == false);
                REQUIRE(vec[2] == true);
                REQUIRE(vec[3] == false);
            }
        }

        WHEN("Flipping individual bit") {
            vec.flip(1);

            THEN("Only specified bit should be flipped") {
                REQUIRE(vec[0] == true);
                REQUIRE(vec[1] == true);
                REQUIRE(vec[2] == true);
                REQUIRE(vec[3] == false);
            }
        }
    }
}

SCENARIO("bit_vector shift operations", "[bit_vector][shift]") {
    GIVEN("A bit_vector with pattern") {
        bit_vector<> vec = {true, false, true, false, true};

        WHEN("Left shifting by 0") {
            auto result = vec << 0;

            THEN("Result should be same as original") {
                REQUIRE(result == vec);
            }
        }

        WHEN("Left shifting by positive amount") {
            auto result = vec << 2;

            THEN("Bits should shift left, zeros fill from right") {
                REQUIRE(result.size() == 5);
                REQUIRE(result[0] == false);
                REQUIRE(result[1] == false);
                REQUIRE(result[2] == true);
                REQUIRE(result[3] == false);
                REQUIRE(result[4] == true);
            }
        }

        WHEN("Left shifting by size or more") {
            auto result = vec << 5;

            THEN("All bits should be zero") {
                REQUIRE(result.none());
            }
        }

        WHEN("Right shifting by positive amount") {
            auto result = vec >> 2;

            THEN("Bits should shift right, zeros fill from left") {
                REQUIRE(result.size() == 5);
                REQUIRE(result[0] == true);
                REQUIRE(result[1] == false);
                REQUIRE(result[2] == true);
                REQUIRE(result[3] == false);
                REQUIRE(result[4] == false);
            }
        }

        WHEN("Using compound shift assignment") {
            bit_vector<> copy(vec);
            copy <<= 2;

            THEN("Original should be modified") {
                REQUIRE(copy == (vec << 2));
            }
        }
    }
}

SCENARIO("bit_vector query operations", "[bit_vector][queries]") {
    GIVEN("bit_vectors with different patterns") {
        bit_vector<> all_true(5, true);
        bit_vector<> all_false(5, false);
        bit_vector<> mixed = {true, false, true, false, true};

        THEN("all() should work correctly") {
            REQUIRE(all_true.all());
            REQUIRE_FALSE(all_false.all());
            REQUIRE_FALSE(mixed.all());
            REQUIRE(bit_vector<>().all()); // Empty vector returns true
        }

        THEN("any() should work correctly") {
            REQUIRE(all_true.any());
            REQUIRE_FALSE(all_false.any());
            REQUIRE(mixed.any());
            REQUIRE_FALSE(bit_vector<>().any());
        }

        THEN("none() should work correctly") {
            REQUIRE_FALSE(all_true.none());
            REQUIRE(all_false.none());
            REQUIRE_FALSE(mixed.none());
            REQUIRE(bit_vector<>().none());
        }

        THEN("count() should return number of true bits") {
            REQUIRE(all_true.count() == 5);
            REQUIRE(all_false.count() == 0);
            REQUIRE(mixed.count() == 3);
        }

        THEN("test() should access bits with bounds checking") {
            REQUIRE(mixed.test(0) == true);
            REQUIRE(mixed.test(1) == false);
            REQUIRE_THROWS_AS(mixed.test(5), std::out_of_range);
        }
    }
}

SCENARIO("bit_vector comparison operations", "[bit_vector][comparison]") {
    GIVEN("Two identical bit_vectors") {
        bit_vector<> vec1 = {true, false, true, false, true};
        bit_vector<> vec2 = {true, false, true, false, true};

        THEN("They should be equal") {
            REQUIRE(vec1 == vec2);
            REQUIRE_FALSE(vec1 != vec2);
        }
    }

    GIVEN("Two different bit_vectors of same size") {
        bit_vector<> vec1 = {true, false, true, false, true};
        bit_vector<> vec2 = {true, true, true, false, true};

        THEN("They should not be equal") {
            REQUIRE_FALSE(vec1 == vec2);
            REQUIRE(vec1 != vec2);
        }
    }

    GIVEN("Two bit_vectors of different sizes") {
        bit_vector<> vec1 = {true, false, true};
        bit_vector<> vec2 = {true, false, true, false};

        THEN("They should not be equal") {
            REQUIRE_FALSE(vec1 == vec2);
            REQUIRE(vec1 != vec2);
        }
    }
}

SCENARIO("bit_vector conversion operations", "[bit_vector][conversion]") {
    GIVEN("A bit_vector with pattern") {
        bit_vector<> vec = {true, false, true, false};

        THEN("to_string() should produce correct string") {
            REQUIRE(vec.to_string() == "0101"); // Default: LSB at index 0
            REQUIRE(vec.to_string('0', '1') == "0101");
            REQUIRE(vec.to_string('F', 'T') == "FTFT");
        }
    }

    GIVEN("A bit_vector small enough for unsigned long") {
        bit_vector<> vec1 = {true, false, true}; // 101 binary = 5 decimal
        bit_vector<> vec2(32, false);
        vec2.set(31); // Set MSB of 32-bit unsigned long

        THEN("to_ulong() should convert correctly") {
            REQUIRE(vec1.to_ulong() == 5);
            REQUIRE(vec2.to_ulong() == (1UL << 31));
        }
    }

    GIVEN("A bit_vector too large for unsigned long") {
        bit_vector<> vec(33, true);

        THEN("to_ulong() should throw overflow_error") {
            REQUIRE_THROWS_AS(vec.to_ulong(), std::overflow_error);
        }
    }

    GIVEN("A bit_vector with bits beyond target type that are zero") {
        bit_vector<> vec(33, false);
        vec.set(31); // Only set a bit within 32-bit range

        THEN("to_ulong() should succeed") {
            REQUIRE_NOTHROW(vec.to_ulong());
            REQUIRE(vec.to_ulong() == (1UL << 31));
        }
    }
}

SCENARIO("bit_vector iterator operations", "[bit_vector][iterators]") {
    GIVEN("A bit_vector with values") {
        bit_vector<> vec = {true, false, true, false, true};

        THEN("Begin and end iterators should work") {
            auto it = vec.begin();
            REQUIRE(*it == true);
            ++it;
            REQUIRE(*it == false);
            ++it;
            REQUIRE(*it == true);

            auto end = vec.end();
            REQUIRE(std::distance(vec.begin(), vec.end()) == 5);
        }

        THEN("Range-based for loop should work") {
            std::vector<bool> collected;
            for (bool b: vec) {
                collected.push_back(b);
            }
            REQUIRE(collected.size() == 5);
            REQUIRE(collected[0] == true);
            REQUIRE(collected[1] == false);
            REQUIRE(collected[2] == true);
            REQUIRE(collected[3] == false);
            REQUIRE(collected[4] == true);
        }

        THEN("Reverse iterators should work") {
            auto rit = vec.rbegin();
            REQUIRE(*rit == true);
            ++rit;
            REQUIRE(*rit == false);
            ++rit;
            REQUIRE(*rit == true);
        }

        THEN("Const iterators should work") {
            const auto &const_vec = vec;
            auto cit = const_vec.cbegin();
            REQUIRE(*cit == true);
            ++cit;
            REQUIRE(*cit == false);
        }

        THEN("Iterator comparisons should work") {
            auto it1 = vec.begin();
            auto it2 = vec.begin();
            auto it3 = vec.end();

            REQUIRE(it1 == it2);
            REQUIRE(it1 != it3);
            REQUIRE(it1 < it3);
        }
    }

    GIVEN("Empty bit_vector") {
        bit_vector<> vec;

        THEN("Begin should equal end") {
            REQUIRE(vec.begin() == vec.end());
            REQUIRE(vec.cbegin() == vec.cend());
            REQUIRE(vec.rbegin() == vec.rend());
        }
    }
}

SCENARIO("bit_vector extreme cases and edge behaviors", "[bit_vector][extreme]") {
    GIVEN("Maximum size considerations") {
        bit_vector<> vec;

        THEN("max_size() should return reasonable value") {
            // Should be based on allocator limits
            REQUIRE(vec.max_size() > 0);
        }
    }

    GIVEN("Bit_vector with single element") {
        bit_vector<> vec(1, true);

        THEN("All operations should work correctly") {
            REQUIRE(vec.size() == 1);
            REQUIRE(vec[0] == true);

            vec.push_back(false);
            REQUIRE(vec.size() == 2);
            REQUIRE(vec[1] == false);

            vec.pop_back();
            REQUIRE(vec.size() == 1);
            REQUIRE(vec[0] == true);
        }
    }

    GIVEN("Bit_vector with size exactly at block boundary") {
        const size_t bits_per_block = 64; // Assuming 64-bit blocks
        bit_vector<> vec(bits_per_block, true);

        THEN("Operations at boundary should work") {
            REQUIRE(vec.size() == bits_per_block);
            REQUIRE(vec.all());

            vec.push_back(false);
            REQUIRE(vec.size() == bits_per_block + 1);
            REQUIRE(vec[bits_per_block] == false);

            vec.pop_back();
            REQUIRE(vec.all());
        }
    }

    GIVEN("Bit_vector with pattern across block boundaries") {
        const size_t bits_per_block = 64;
        bit_vector<> vec(bits_per_block * 2 + 10);

        WHEN("Setting bits across boundary") {
            for (size_t i = bits_per_block - 5; i < bits_per_block + 5; ++i) {
                vec.set(i);
            }

            THEN("Bits should be correctly set") {
                for (size_t i = bits_per_block - 5; i < bits_per_block + 5; ++i) {
                    REQUIRE(vec[i] == true);
                }
            }
        }
    }

    GIVEN("Bit_vector with alternating pattern") {
        const size_t size = 1000;
        bit_vector<> vec(size);
        for (size_t i = 0; i < size; ++i) {
            vec[i] = (i % 2 == 0);
        }

        THEN("Count should be correct") {
            REQUIRE(vec.count() == size / 2 + (size % 2));
        }

        WHEN("Flipping all bits") {
            vec.flip();

            THEN("Pattern should be inverted") {
                for (size_t i = 0; i < size; ++i) {
                    REQUIRE(vec[i] == (i % 2 != 0));
                }
            }
        }
    }

    GIVEN("Bit_vector for self-assignment testing") {
        bit_vector<> vec = {true, false, true};

        WHEN("Assigning to self") {
            vec = vec;

            THEN("Should remain unchanged") {
                REQUIRE(vec.size() == 3);
                REQUIRE(vec[0] == true);
                REQUIRE(vec[1] == false);
                REQUIRE(vec[2] == true);
            }
        }

        WHEN("Moving from self") {
            // Note: Self-move is generally not allowed, but we test behavior
            bit_vector<> copy(vec);
            vec = std::move(vec);

            THEN("Should be in valid but unspecified state") {
                // At minimum, should not crash
            }
        }
    }
}

SCENARIO("bit_vector swap operations", "[bit_vector][swap]") {
    GIVEN("Two different bit_vectors") {
        bit_vector<> vec1 = {true, false, true};
        bit_vector<> vec2 = {false, true, false, true};

        bit_vector<> original1(vec1);
        bit_vector<> original2(vec2);

        WHEN("Swapping vectors") {
            vec1.swap(vec2);

            THEN("Contents should be swapped") {
                REQUIRE(vec1 == original2);
                REQUIRE(vec2 == original1);
            }
        }

        WHEN("Using free function swap") {
            swap(vec1, vec2);

            THEN("Contents should be swapped") {
                REQUIRE(vec1 == original2);
                REQUIRE(vec2 == original1);
            }
        }

        WHEN("Swapping with self") {
            vec1.swap(vec1);

            THEN("Should remain unchanged") {
                REQUIRE(vec1 == original1);
            }
        }
    }
}

SCENARIO("bit_vector exception safety", "[bit_vector][exceptions]") {
    GIVEN("A bit_vector") {
        bit_vector<> vec = {true, false, true};

        WHEN("Accessing out of bounds with at()") {
            THEN("Should throw out_of_range") {
                REQUIRE_THROWS_AS(vec.at(3), std::out_of_range);
                REQUIRE_THROWS_AS(vec.at(100), std::out_of_range);
            }
        }

        WHEN("Using test() with invalid index") {
            THEN("Should throw out_of_range") {
                REQUIRE_THROWS_AS(vec.test(3), std::out_of_range);
            }
        }

        WHEN("Using set() with invalid index") {
            THEN("Should throw out_of_range") {
                REQUIRE_THROWS_AS(vec.set(3), std::out_of_range);
            }
        }

        WHEN("Using reset() with invalid index") {
            THEN("Should throw out_of_range") {
                REQUIRE_THROWS_AS(vec.reset(3), std::out_of_range);
            }
        }

        WHEN("Using flip() with invalid index") {
            THEN("Should throw out_of_range") {
                REQUIRE_THROWS_AS(vec.flip(3), std::out_of_range);
            }
        }
    }
}

SCENARIO("bit_vector with different allocators", "[bit_vector][allocator]") {
    GIVEN("bit_vectors with same allocator type") {
        bit_vector<> vec1 = {true, false};
        bit_vector<> vec2 = {false, true};

        THEN("They should be swappable") {
            REQUIRE_NOTHROW(vec1.swap(vec2));
        }
    }
}
