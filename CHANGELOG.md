# Changelog

## [2.0.0](https://github.com/rainy-juzixiao/rainy-toolkit/compare/v1.2.0...v2.0.0) (2026-08-29)


### ⚠ BREAKING CHANGES

* **exceptions:** Removed decay_t wrapper when constructing exception objects in exception_handler function, now using Except directly.

### Features

* Added pred edition for equal algo ([1bf3720](https://github.com/rainy-juzixiao/rainy-toolkit/commit/1bf37209617fd516af960ed38f826813c71d4d16))
* Added recycling_allocator and test case. ([ce853e7](https://github.com/rainy-juzixiao/rainy-toolkit/commit/ce853e75287ec94bdc15890055e1402a411fd8fe))
* Added shared_ptr and test case. ([450f793](https://github.com/rainy-juzixiao/rainy-toolkit/commit/450f7938591525e96a3569c0be379a684cd1d176))
* Added tss_ptr and test case also added thread_context. ([e1e10d6](https://github.com/rainy-juzixiao/rainy-toolkit/commit/e1e10d66c9b6b2afcc1c34019453252331c540bf))
* **algorithm,views:** Add binary search operations ([fba5fb0](https://github.com/rainy-juzixiao/rainy-toolkit/commit/fba5fb05eb5baffe4d7db7817d307759ed75d5e3))
* **algorithm,views:** Add heap operations ([aad5597](https://github.com/rainy-juzixiao/rainy-toolkit/commit/aad55979dd3e29791141d85b33648b78a6b1e989))
* **algorithm,views:** Add minimum/maximum and permutation operations ([b9b837b](https://github.com/rainy-juzixiao/rainy-toolkit/commit/b9b837b8700795ead763319ddd0fa5090b8a02cc))
* **algorithm,views:** Add modifying sequence operations ([76a5ab9](https://github.com/rainy-juzixiao/rainy-toolkit/commit/76a5ab9c3748fc876526c15dc89600478161a6af))
* **algorithm,views:** Add non-modifying sequence operations ([4631228](https://github.com/rainy-juzixiao/rainy-toolkit/commit/463122885a1662203b1968a81c038a6fc7ba2fc8))
* **algorithm,views:** Add partitioning operations ([c4467b4](https://github.com/rainy-juzixiao/rainy-toolkit/commit/c4467b47728ba73e1ec33aa5d38daad64a518b6f))
* **algorithm,views:** Add set operations ([f95fa7b](https://github.com/rainy-juzixiao/rainy-toolkit/commit/f95fa7b48a632115256644a9fcb55195a1d52326))
* **algorithm,views:** Add sorting operations ([45d5235](https://github.com/rainy-juzixiao/rainy-toolkit/commit/45d5235f5a8c25f34dfc2c1152570c127690ba20))
* **collections,algorithm:** Add lazy replace views and eager replace algorithms ([29a3df5](https://github.com/rainy-juzixiao/rainy-toolkit/commit/29a3df5339da97e3c1292dfe9a38073c0aaaa024))
* **core:** add atomic test suite and fix volatile/generic atomic support ([4380322](https://github.com/rainy-juzixiao/rainy-toolkit/commit/4380322d00b528b865787d693ae39b440ee8e900))
* **core:** add constexpr typeinfo system with inheritance registry ([7547b4a](https://github.com/rainy-juzixiao/rainy-toolkit/commit/7547b4a8900668a507d09e6163a33dbf3d43c6f8))
* **core:** add platform abstraction layer for threading primitives ([91783e1](https://github.com/rainy-juzixiao/rainy-toolkit/commit/91783e1003ae426a339891268a3b086ec62404b9))
* **core:** Added functional library for core. ([5fb0fd5](https://github.com/rainy-juzixiao/rainy-toolkit/commit/5fb0fd5d2d36f69abffbd912edf2f14dcc7b40c3))
* **core:** Added new exception class nullpointer_exception and throw func. ([a09a4c2](https://github.com/rainy-juzixiao/rainy-toolkit/commit/a09a4c27c52c02e310a6ec072f7f4c6db97a85dd))
* **core:** Added new feature enumeration and moon annotations and added test case ([3683e08](https://github.com/rainy-juzixiao/rainy-toolkit/commit/3683e08ab4ede55374863e2fafea5345a63f8260))
* **exceptions:** add comprehensive exception handling framework ([339f783](https://github.com/rainy-juzixiao/rainy-toolkit/commit/339f7831a9e6c5a76f8a87c8ff524ce4292b31a6))
* **platform:** implement new platform layers. ([5f703a7](https://github.com/rainy-juzixiao/rainy-toolkit/commit/5f703a7b834d3e408ffdbd7c81d639647fbcde5e))


### Bug Fixes

* **annotations:** correct exception handling and template consistency in lifetime annotations ([e58cc9f](https://github.com/rainy-juzixiao/rainy-toolkit/commit/e58cc9f19d43038f365aac038d3096f506e982bb))
* **cmake:** Fix option ([b45205b](https://github.com/rainy-juzixiao/rainy-toolkit/commit/b45205b675e01e9cd2c379a06ec9007a3e3319f0))
* **collections:** Fix never-compiled views headers ([298a15a](https://github.com/rainy-juzixiao/rainy-toolkit/commit/298a15ac15c472337bb84ac76cdc5df0bafdfb3e))
* **collections:** Stop transform_view from moving out of lvalue ranges ([acfda22](https://github.com/rainy-juzixiao/rainy-toolkit/commit/acfda22be3170f6182f4fe94937979225e216504))
* **container:** Fix indirect and movable_box found by new tests ([5719049](https://github.com/rainy-juzixiao/rainy-toolkit/commit/571904941682c77982c8144a113fb7ea30836aba))
* Fix compile error. ([144c959](https://github.com/rainy-juzixiao/rainy-toolkit/commit/144c9593ffdf7636818ab22bc5af3f323669900c))
* fix compile issues in hazard_pointer.cc ([24648fd](https://github.com/rainy-juzixiao/rainy-toolkit/commit/24648fd86dfa3771590ee1a544f7b2cd3c82f4c0))
* Fix exception throw function throw_runtime_error not found ([b2fcfc7](https://github.com/rainy-juzixiao/rainy-toolkit/commit/b2fcfc7189bd145e90459d92fca3a996d28672b1))
* Fix some issues with container. ([d688d06](https://github.com/rainy-juzixiao/rainy-toolkit/commit/d688d0669695b43cf3d5df2f11fa2aeba5be1932))
* Fix tss_ptr.cc compile error. ([bde733a](https://github.com/rainy-juzixiao/rainy-toolkit/commit/bde733a9512895b13c7e5e391e6fe22ff91d9d69))
* **test:** Fix format test case in cxx26 ([1a840cc](https://github.com/rainy-juzixiao/rainy-toolkit/commit/1a840ccef5b23ae15df9bb6721155a31fcaba6d8))
* **test:** Fix functor test case. ([19871ec](https://github.com/rainy-juzixiao/rainy-toolkit/commit/19871eca8efd7492feff0e2e43444d00ecbe89b4))
* **test:** Fix tuple test case in cxx26 ([dbd03a9](https://github.com/rainy-juzixiao/rainy-toolkit/commit/dbd03a9ad57e155eb0b490baf0c28c00f6e4ed11))
* **type_traits:** Fix bugs found by new type_traits tests ([a606935](https://github.com/rainy-juzixiao/rainy-toolkit/commit/a6069351b90b015199f07980bf44204bab66be09))
* **utility:** Add missing includes for bit and reverse_iterator ([493a8ad](https://github.com/rainy-juzixiao/rainy-toolkit/commit/493a8ad2be716348fe9e7afb6f7f1081bead601b))


### Code Refactoring

* **exceptions:** improve exception implementation and fix semantic issues ([984847d](https://github.com/rainy-juzixiao/rainy-toolkit/commit/984847db378f01871561e593cca06c2753702299))

## [1.2.0](https://github.com/rainy-juzixiao/rainy-toolkit/compare/v1.1.1...v1.2.0) (2026-07-30)


### Features

* **algorithm:** Added new algorithm and test case. ([cc78208](https://github.com/rainy-juzixiao/rainy-toolkit/commit/cc78208b7fc3f4fe90d2f535db23cdc626cdc714))
* **chore:** Added diagnostics library. ([b9e0742](https://github.com/rainy-juzixiao/rainy-toolkit/commit/b9e07427869ab6a71da7ea127b6a635941e53704))
* **container:** Enhance tuple and variant with allocator support ([c3a7fbe](https://github.com/rainy-juzixiao/rainy-toolkit/commit/c3a7fbeb63a115a3d4a431ca23ed39e52e4d435c))
* **core:** Add memory library ([615fae2](https://github.com/rainy-juzixiao/rainy-toolkit/commit/615fae208e8d1e096f5c69555fd182887af3f209))
* **core:** Add type_traits library. ([838093a](https://github.com/rainy-juzixiao/rainy-toolkit/commit/838093af75c6588911b065f89b3221140a9e70f2))
* **core:** add utility core library. ([18ecb85](https://github.com/rainy-juzixiao/rainy-toolkit/commit/18ecb85ea9b64dde85a21040987262bbc78c0ddb))
* **core:** Added atomic operations and filesystem operations, also fix some compile errors with use crosscompile from linux to windows. ([9734a80](https://github.com/rainy-juzixiao/rainy-toolkit/commit/9734a8025e8ddfe4bedad87b6d1e27276f689c0d))
* **core:** Added collections library. ([bee8178](https://github.com/rainy-juzixiao/rainy-toolkit/commit/bee8178a99c42fac4fbac6de771e865d2d3fe341))
* **core:** Added container library. ([78ee73e](https://github.com/rainy-juzixiao/rainy-toolkit/commit/78ee73e837d7de3d9922e03def31593458b669fe))
* **core:** Added temporary_buffer for memory. ([af636ff](https://github.com/rainy-juzixiao/rainy-toolkit/commit/af636fff1b2a2a17b361032d504278625be70c27))
* **core:** Added text library. ([fbf4333](https://github.com/rainy-juzixiao/rainy-toolkit/commit/fbf43338487c877b31b6022ee265a68c2eebbb63))
* **core:** Fix some issues and remove add_rainy_library and add_rainy_interface_library, replaced by rainy_configure_target ([5850fa2](https://github.com/rainy-juzixiao/rainy-toolkit/commit/5850fa2e8f5dc14495aaaf84cd15a83e9dd84ffb))
* **memory:** add destroy and construct method for allocator. ([de3b73d](https://github.com/rainy-juzixiao/rainy-toolkit/commit/de3b73dbacf6b448527e61c7c68efdc9c9d6fc73))
* **memory:** add max_size method. ([4668d2e](https://github.com/rainy-juzixiao/rainy-toolkit/commit/4668d2e1869d7c49816a15f05bab1dbb883e883f))
* **platform:** add ctz function. ([917b5ac](https://github.com/rainy-juzixiao/rainy-toolkit/commit/917b5ac2a29b4ed44e3cb6e6cd7b23a09ecf12bb))
* **poly:** Add poly library with basic_poly, CMake config, and unit tests ([7736cc7](https://github.com/rainy-juzixiao/rainy-toolkit/commit/7736cc777925278c6e92bbaa99d98bfc6b625f3f))
* **sleepy:** add __MUZIYAN_IS_HERE__ macro and ignored_namespaces config ([3c38125](https://github.com/rainy-juzixiao/rainy-toolkit/commit/3c38125c1ebc012ce50334ba2dcf0a3dedb09b16))
* **sleepy:** add --root arg and improve diagnostic output ([a58f582](https://github.com/rainy-juzixiao/rainy-toolkit/commit/a58f582212296b0de0a9aebd4fcd812d847f02b8))
* **sleepy:** add [@module](https://github.com/module) tag support and VitePress reference site generator ([a835ce0](https://github.com/rainy-juzixiao/rainy-toolkit/commit/a835ce048a9ad5c731f4e8621cce059598a27cc5))
* **sleepy:** add free function overload resolution ([da315b3](https://github.com/rainy-juzixiao/rainy-toolkit/commit/da315b325f8ab8b7895faddad1d880baa45b5658))
* **sleepy:** add variable template and constant support ([c829ec2](https://github.com/rainy-juzixiao/rainy-toolkit/commit/c829ec2b760e29c58afb75b552aeb85afd0d9b4f))
* **sleepy:** split enums and enum classes in generated docs ([0ab64ff](https://github.com/rainy-juzixiao/rainy-toolkit/commit/0ab64fffb96ae93b630e671c85296272fadde06c))
* **utility:** Added more hash support. ([7f9fd78](https://github.com/rainy-juzixiao/rainy-toolkit/commit/7f9fd786bed735879e20dcd2d6f23013f3c78bc6))


### Bug Fixes

* **algorithm:** Fix compile issues. ([9edf725](https://github.com/rainy-juzixiao/rainy-toolkit/commit/9edf725836d905ed5343871c0eda31f629684490))
* **collections:** replace or to || ([5c25f5c](https://github.com/rainy-juzixiao/rainy-toolkit/commit/5c25f5c1187e50f25de9bc83e5113462337f51d0))
* **container:** Fix ambiguous in implements::get_val ([7c4fba5](https://github.com/rainy-juzixiao/rainy-toolkit/commit/7c4fba5ceba02d344f5ef504e01d419de6b3173e))
* **container:** Fix compile issues. ([3bca42f](https://github.com/rainy-juzixiao/rainy-toolkit/commit/3bca42f5a79b0821e3d3e0ba202e899936240f58))
* **core:** Added monostate for platform.hpp and refactor to fix test case error in temporary_buffer.cc. ([f39408e](https://github.com/rainy-juzixiao/rainy-toolkit/commit/f39408eb323cbb38fde4e2f49d75e46dad5c93e4))
* **core:** fallback remove add_rainy_interface_library. ([c4fc933](https://github.com/rainy-juzixiao/rainy-toolkit/commit/c4fc9332f79ad894eb3859003eb05daad2cd1328))
* **diagnostics:** Fix compile errors. ([2365780](https://github.com/rainy-juzixiao/rainy-toolkit/commit/236578025b28698309f9f6baebf2cd869b543202))
* **diagnostics:** Fix source location error. ([b9e4d98](https://github.com/rainy-juzixiao/rainy-toolkit/commit/b9e4d983155504f9c6e5e0298414eaeaacb8d809))
* enforce ARM64 check. ([0a0ee90](https://github.com/rainy-juzixiao/rainy-toolkit/commit/0a0ee9088510dd72cdf5318c484dfd7fb5d322fd))
* **exceptions:** Fix link error. ([d41924d](https://github.com/rainy-juzixiao/rainy-toolkit/commit/d41924dcf844b4c6ac083dc5ec76da14b0e4bc86))
* fix properties.hpp ([b9c551c](https://github.com/rainy-juzixiao/rainy-toolkit/commit/b9c551ce8c3bf4412345d732e96364cfd929be1c))
* Fix rainy-toolkit-core-unit-test ([4c22243](https://github.com/rainy-juzixiao/rainy-toolkit/commit/4c2224303d5606237067a00a30c7f8912bddf4e5))
* Fix system_info.cxx ([4224009](https://github.com/rainy-juzixiao/rainy-toolkit/commit/422400977ac8959d6a41d41bc127c521a8425e26))
* **memory:** Ensure max_size is constexpr from cxx20. ([1063ff6](https://github.com/rainy-juzixiao/rainy-toolkit/commit/1063ff64c705051f9ff874bf415cb31e36a15de5))
* **memory:** fix some compile errors. ([3c35b2d](https://github.com/rainy-juzixiao/rainy-toolkit/commit/3c35b2dda2256fca0ac28774f0d7469f9596697f))
* **platform:** add rainy::utility::monostate type and fix implicit declaration in filesystem.cc on POSIX ([8651cbf](https://github.com/rainy-juzixiao/rainy-toolkit/commit/8651cbf00c5229965cfb3e9628ce84e964709ba4))
* **platform:** Fix system_info.cxx compile issues in windows. ([add5932](https://github.com/rainy-juzixiao/rainy-toolkit/commit/add5932310a4b75246e87f5911dc9069d37550cf))
* **sleepy:** Fix escape for markdown generator. ([f7b1ac5](https://github.com/rainy-juzixiao/rainy-toolkit/commit/f7b1ac5871a5850fea681251ea652e37ac710188))
* **type_traits:** fix some compile issues. and remove some internal function. ([edda8bb](https://github.com/rainy-juzixiao/rainy-toolkit/commit/edda8bbb0efbd269a90f79cfb454839f2d3e1edf))
* **utility:** Fix compile issues. ([44dd101](https://github.com/rainy-juzixiao/rainy-toolkit/commit/44dd101b94d6793a9e04ee72381c8aa17d073f72))

## [1.1.1](https://github.com/rainy-juzixiao/rainy-toolkit/compare/v1.1.0...v1.1.1) (2026-05-20)


### Bug Fixes

* Fix compile issues in C++23 for stacktrace.hpp ([41703bd](https://github.com/rainy-juzixiao/rainy-toolkit/commit/41703bd4279c1a040251503e13d93e9c68198620))

## [1.1.0](https://github.com/rainy-juzixiao/rainy-toolkit/compare/v1.0.0...v1.1.0) (2026-05-19)


### Features

* Refactor core components and remove unused files and headers ([d83355d](https://github.com/rainy-juzixiao/rainy-toolkit/commit/d83355df55d03085135e19750016b0a7b4020bac))

## [1.0.0](https://github.com/rainy-juzixiao/rainy-toolkit/compare/v0.8.5...v1.0.0) (2026-05-16)


### ⚠ BREAKING CHANGES

* Added full io component.

### Features

* Added full io component. ([3663992](https://github.com/rainy-juzixiao/rainy-toolkit/commit/3663992f67cee1c75643c3d47002ed236c537dcf))

## [0.8.5](https://github.com/rainy-juzixiao/rainy-toolkit/compare/v0.8.4...v0.8.5) (2026-05-05)


### Bug Fixes

* fix compiler issues in cpp23 and in macOS environment ([203d7a8](https://github.com/rainy-juzixiao/rainy-toolkit/commit/203d7a87ff9cc9915b5cb4cc8ee67bb556d8bbeb))

## [0.8.4](https://github.com/rainy-juzixiao/rainy-toolkit/compare/v0.8.3...v0.8.4) (2026-05-04)


### Bug Fixes

* Added commit check in auto_tag.yml ([ceebb33](https://github.com/rainy-juzixiao/rainy-toolkit/commit/ceebb337e525a231a37317a616f2f96190b2db15))

## [0.8.3](https://github.com/rainy-juzixiao/rainy-toolkit/compare/v0.8.2...v0.8.3) (2026-05-04)


### Bug Fixes

* to fix auto_tag.yml ([c8446b9](https://github.com/rainy-juzixiao/rainy-toolkit/commit/c8446b987a857d4ba30f2c281f9a1e88e45206e0))

## [0.8.2](https://github.com/rainy-juzixiao/rainy-toolkit/compare/v0.8.1...v0.8.2) (2026-05-04)


### Bug Fixes

* to fix auto_tag.yml ([8801940](https://github.com/rainy-juzixiao/rainy-toolkit/commit/8801940281cd12cb1913a7c7e0b76716d49e6ff8))

## [0.8.1](https://github.com/rainy-juzixiao/rainy-toolkit/compare/v0.8.0...v0.8.1) (2026-05-04)


### Bug Fixes

* to fix document report.md(patch 1) ([d2f4568](https://github.com/rainy-juzixiao/rainy-toolkit/commit/d2f4568a869244beaf2d270377478ce1f4d56e6b))

## [0.8.0](https://github.com/rainy-juzixiao/rainy-toolkit/compare/v0.7.4...v0.8.0) (2026-05-04)


### Features

* Added macOS support and added cxx26 reflection support ([5926a0a](https://github.com/rainy-juzixiao/rainy-toolkit/commit/5926a0a5e048f964b2c12fa32d66115240b84241))
* Merge pull request [#45](https://github.com/rainy-juzixiao/rainy-toolkit/issues/45) from rainy-juzixiao/revert-44-develop ([5926a0a](https://github.com/rainy-juzixiao/rainy-toolkit/commit/5926a0a5e048f964b2c12fa32d66115240b84241))
