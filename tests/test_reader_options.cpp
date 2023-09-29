#include <filesystem>

#include <catch2/catch.hpp>

#include <morphio/morphology.h>
#include <morphio/mut/morphology.h>


TEST_CASE("root bifurcation", "[loader_edge_cases]") {
    std::vector<std::string> exts{"asc", "h5", "swc"};
    for (auto&& ext : exts) {
        SECTION("Testing mutable morphology with " + ext + " extension") {
            auto mutable_morph_default_opts = morphio::mut::Morphology(
                "data/edge_cases/root_bifurcation." + ext);

            if (ext == "swc") {
                // By default, root bifurcations can be not loaded from SWC files
                REQUIRE(mutable_morph_default_opts.rootSections().size() == 1);
                REQUIRE(mutable_morph_default_opts.rootSections()[0]->points().size() == 3);
            } else if (ext == "asc" || ext == "h5") {
                // By default, root bifurcations can be loaded from ASC and H5 files
                REQUIRE(mutable_morph_default_opts.rootSections().size() == 3);
                REQUIRE(mutable_morph_default_opts.rootSections()[0]->points().size() == 1);
                REQUIRE(mutable_morph_default_opts.rootSections()[1]->points().size() == 1);
                REQUIRE(mutable_morph_default_opts.rootSections()[2]->points().size() == 3);
            } else {
                throw std::invalid_argument("Unknown extension: " + ext);
            }

            auto mutable_morph =
                morphio::mut::Morphology("data/edge_cases/root_bifurcation." + ext,
                                         morphio::enums::Option::ALLOW_ROOT_BIFURCATIONS);

            REQUIRE(mutable_morph.rootSections().size() == 3);
            REQUIRE(mutable_morph.rootSections()[0]->points().size() == 1);
            REQUIRE(mutable_morph.rootSections()[1]->points().size() == 1);
            REQUIRE(mutable_morph.rootSections()[2]->points().size() == 3);
        }

        SECTION("Testing immutable morphology with " + ext + " extension") {
            auto immutable_morph_default_opts = morphio::Morphology(
                "data/edge_cases/root_bifurcation." + ext);

            if (ext == "swc") {
                // By default, root bifurcations can be not loaded from SWC files
                REQUIRE(immutable_morph_default_opts.rootSections().size() == 1);
                REQUIRE(immutable_morph_default_opts.rootSections()[0].points().size() == 3);
            } else if (ext == "asc" || ext == "h5") {
                // By default, root bifurcations can be loaded from ASC and H5 files
                REQUIRE(immutable_morph_default_opts.rootSections().size() == 3);
                REQUIRE(immutable_morph_default_opts.rootSections()[0].points().size() == 1);
                REQUIRE(immutable_morph_default_opts.rootSections()[1].points().size() == 1);
                REQUIRE(immutable_morph_default_opts.rootSections()[2].points().size() == 3);
            } else {
                throw std::invalid_argument("Unknown extension: " + ext);
            }

            auto immutable_morph =
                morphio::Morphology("data/edge_cases/root_bifurcation." + ext,
                                    morphio::enums::Option::ALLOW_ROOT_BIFURCATIONS);

            REQUIRE(immutable_morph.rootSections().size() == 3);
            REQUIRE(immutable_morph.rootSections()[0].points().size() == 1);
            REQUIRE(immutable_morph.rootSections()[1].points().size() == 1);
            REQUIRE(immutable_morph.rootSections()[2].points().size() == 3);
        }
    }
}


TEST_CASE("soma bifurcation", "[loader_edge_cases]") {
    std::vector<std::string> exts{"asc", "swc"};
    for (auto&& ext : exts) {
        SECTION("testing mutable morphology with " + ext + " extension") {
            // By default, bifurcations in soma are considered as errors in ASC and SWC files
            REQUIRE_THROWS(morphio::mut::Morphology("data/edge_cases/soma_bifurcation." + ext));

            // Bifurcations in soma can optionaly be loaded from ASC and SWC files
            auto mutable_morph =
                morphio::mut::Morphology("data/edge_cases/soma_bifurcation." + ext,
                                         morphio::enums::Option::ALLOW_SOMA_BIFURCATIONS);

            REQUIRE(mutable_morph.soma()->points().size() == 8);
            REQUIRE(mutable_morph.rootSections().size() == 1);
        }

        SECTION("testing immutable morphology with " + ext + " extension") {
            // By default, bifurcations in soma are considered as errors in ASC and SWC files
            REQUIRE_THROWS(morphio::Morphology("data/edge_cases/soma_bifurcation." + ext));

            // Bifurcations in soma can optionaly be loaded from ASC and SWC files
            auto immutable_morph =
                morphio::Morphology("data/edge_cases/soma_bifurcation." + ext,
                                    morphio::enums::Option::ALLOW_SOMA_BIFURCATIONS);

            REQUIRE(immutable_morph.soma().points().size() == 8);
            REQUIRE(immutable_morph.rootSections().size() == 1);
        }
    }
    SECTION("testing mutable morphology with h5 extension") {
        // Bifurcations in soma are always considered as errors in H5 files
        REQUIRE_THROWS(morphio::mut::Morphology("data/soma_bifurcation.h5"));
        REQUIRE_THROWS(morphio::mut::Morphology("data/soma_bifurcation.h5",
                                                morphio::enums::Option::ALLOW_SOMA_BIFURCATIONS));
        REQUIRE_THROWS(morphio::Morphology("data/soma_bifurcation.h5"));
        REQUIRE_THROWS(morphio::Morphology("data/soma_bifurcation.h5",
                                           morphio::enums::Option::ALLOW_SOMA_BIFURCATIONS));
    }
}


TEST_CASE("multiple somata", "[loader_edge_cases]") {
    std::vector<std::string> exts{"asc", "swc"};
    for (auto&& ext : exts) {
        SECTION("testing mutable morphology with " + ext + " extension") {
            // By default, multiple somata are considered as errors in ASC and SWC files
            REQUIRE_THROWS(morphio::mut::Morphology("data/multiple_soma." + ext));

            // Multiple somata can optionaly be loaded from ASC and SWC files
            auto mutable_morph =
                morphio::mut::Morphology("data/multiple_soma." + ext,
                                         morphio::enums::Option::ALLOW_MULTIPLE_SOMATA);

            REQUIRE(mutable_morph.soma()->points().size() == 2);
            REQUIRE(mutable_morph.rootSections().size() == 2);
        }

        SECTION("testing immutable morphology with " + ext + " extension") {
            // By default, multiple somata are considered as errors in ASC and SWC files
            REQUIRE_THROWS(morphio::Morphology("data/multiple_soma." + ext));

            // Multiple somata can optionaly be loaded from ASC and SWC files
            auto immutable_morph =
                morphio::Morphology("data/multiple_soma." + ext,
                                    morphio::enums::Option::ALLOW_MULTIPLE_SOMATA);

            REQUIRE(immutable_morph.soma().points().size() == 2);
            REQUIRE(immutable_morph.rootSections().size() == 2);
        }
    }
    SECTION("testing mutable morphology with h5 extension") {
        // Multiple somata are always considered as errors in H5 files
        REQUIRE_THROWS(morphio::mut::Morphology("data/multiple_soma.h5"));
        REQUIRE_THROWS(morphio::mut::Morphology("data/multiple_soma.h5",
                                                morphio::enums::Option::ALLOW_MULTIPLE_SOMATA));
        REQUIRE_THROWS(morphio::Morphology("data/multiple_soma.h5"));
        REQUIRE_THROWS(morphio::Morphology("data/multiple_soma.h5",
                                           morphio::enums::Option::ALLOW_MULTIPLE_SOMATA));
    }
}


TEST_CASE("bad root point", "[loader_edge_cases]") {
    std::vector<std::string> exts{"swc"};
    for (auto&& ext : exts) {
        SECTION("testing mutable morphology with " + ext + " extension") {
            // By default, root points not equal to -1 are considered as errors in SWC files
            REQUIRE_THROWS(morphio::mut::Morphology("data/edge_cases/bad_root_point." + ext));

            // Bad root points can optionaly be loaded from SWC files
            auto mutable_morph =
                morphio::mut::Morphology("data/edge_cases/bad_root_point." + ext,
                                         morphio::enums::Option::ALLOW_CUSTOM_ROOT_ID);

            REQUIRE(mutable_morph.soma()->points().size() == 0);
            REQUIRE(mutable_morph.rootSections().size() == 2);
        }
    }
}


TEST_CASE("conversions", "[loader_edge_cases]") {
    auto tmpDirectory = std::filesystem::temp_directory_path() / "test_reader_option_conversions";
    std::filesystem::create_directories(tmpDirectory);

    SECTION("testing SWC to other formats") {
        SECTION("testing ALLOW_CUSTOM_ROOT_ID option") {
            auto mutable_morph =
                morphio::mut::Morphology("data/edge_cases/bad_root_point.swc",
                                         morphio::enums::Option::ALLOW_CUSTOM_ROOT_ID);

            mutable_morph.write(tmpDirectory / "from_swc_ALLOW_CUSTOM_ROOT_ID.asc");
            morphio::mut::Morphology(tmpDirectory / "from_swc_ALLOW_CUSTOM_ROOT_ID.asc");

            mutable_morph.write(tmpDirectory / "from_swc_ALLOW_CUSTOM_ROOT_ID.h5");
            morphio::mut::Morphology(tmpDirectory / "from_swc_ALLOW_CUSTOM_ROOT_ID.h5");

            mutable_morph.write(tmpDirectory / "from_swc_ALLOW_CUSTOM_ROOT_ID.swc");
            morphio::mut::Morphology(tmpDirectory / "from_swc_ALLOW_CUSTOM_ROOT_ID.swc");
        }

        SECTION("testing ALLOW_MULTIPLE_SOMATA option") {
            auto mutable_morph =
                morphio::mut::Morphology("data/multiple_soma.swc",
                                         morphio::enums::Option::ALLOW_MULTIPLE_SOMATA);

            mutable_morph.write(tmpDirectory / "from_swc_ALLOW_MULTIPLE_SOMATA.asc");
            morphio::mut::Morphology(tmpDirectory / "from_swc_ALLOW_MULTIPLE_SOMATA.asc");

            mutable_morph.write(tmpDirectory / "from_swc_ALLOW_MULTIPLE_SOMATA.h5");
            morphio::mut::Morphology(tmpDirectory / "from_swc_ALLOW_MULTIPLE_SOMATA.h5");

            mutable_morph.write(tmpDirectory / "from_swc_ALLOW_MULTIPLE_SOMATA.swc");
            morphio::mut::Morphology(tmpDirectory / "from_swc_ALLOW_MULTIPLE_SOMATA.swc");
        }

        SECTION("testing ALLOW_SOMA_BIFURCATIONS option") {
            auto mutable_morph =
                morphio::mut::Morphology("data/edge_cases/soma_bifurcation.swc",
                                         morphio::enums::Option::ALLOW_SOMA_BIFURCATIONS);

            mutable_morph.write(tmpDirectory / "from_swc_ALLOW_SOMA_BIFURCATIONS.asc");
            morphio::mut::Morphology(tmpDirectory / "from_swc_ALLOW_SOMA_BIFURCATIONS.asc");

            mutable_morph.write(tmpDirectory / "from_swc_ALLOW_SOMA_BIFURCATIONS.h5");
            morphio::mut::Morphology(tmpDirectory / "from_swc_ALLOW_SOMA_BIFURCATIONS.h5");

            mutable_morph.write(tmpDirectory / "from_swc_ALLOW_SOMA_BIFURCATIONS.swc");
            morphio::mut::Morphology(tmpDirectory / "from_swc_ALLOW_SOMA_BIFURCATIONS.swc");
        }

        SECTION("testing ALLOW_ROOT_BIFURCATIONS option") {
            auto mutable_morph =
                morphio::mut::Morphology("data/edge_cases/root_bifurcation.swc",
                                         morphio::enums::Option::ALLOW_ROOT_BIFURCATIONS);
            REQUIRE_THROWS(
                mutable_morph.write(tmpDirectory / "from_swc_ALLOW_ROOT_BIFURCATIONS.asc"));
            REQUIRE_THROWS(
                mutable_morph.write(tmpDirectory / "from_swc_ALLOW_ROOT_BIFURCATIONS.h5"));
            REQUIRE_THROWS(
                mutable_morph.write(tmpDirectory / "from_swc_ALLOW_ROOT_BIFURCATIONS.swc"));
        }
    }

    SECTION("testing ASC to other formats") {
        SECTION("testing ALLOW_CUSTOM_ROOT_ID option") {
            // Can not be loaded with ASC format
        }

        SECTION("testing ALLOW_MULTIPLE_SOMATA option") {
            auto mutable_morph =
                morphio::mut::Morphology("data/multiple_soma.asc",
                                         morphio::enums::Option::ALLOW_MULTIPLE_SOMATA);

            mutable_morph.write(tmpDirectory / "from_asc_ALLOW_MULTIPLE_SOMATA.asc");
            morphio::mut::Morphology(tmpDirectory / "from_asc_ALLOW_MULTIPLE_SOMATA.asc");

            mutable_morph.write(tmpDirectory / "from_asc_ALLOW_MULTIPLE_SOMATA.h5");
            morphio::mut::Morphology(tmpDirectory / "from_asc_ALLOW_MULTIPLE_SOMATA.h5");

            mutable_morph.write(tmpDirectory / "from_asc_ALLOW_MULTIPLE_SOMATA.swc");
            morphio::mut::Morphology(tmpDirectory / "from_asc_ALLOW_MULTIPLE_SOMATA.swc");
        }

        SECTION("testing ALLOW_SOMA_BIFURCATIONS option") {
            auto mutable_morph =
                morphio::mut::Morphology("data/edge_cases/soma_bifurcation.asc",
                                         morphio::enums::Option::ALLOW_SOMA_BIFURCATIONS);

            mutable_morph.write(tmpDirectory / "from_asc_ALLOW_SOMA_BIFURCATIONS.asc");
            morphio::mut::Morphology(tmpDirectory / "from_asc_ALLOW_SOMA_BIFURCATIONS.asc");

            mutable_morph.write(tmpDirectory / "from_asc_ALLOW_SOMA_BIFURCATIONS.h5");
            morphio::mut::Morphology(tmpDirectory / "from_asc_ALLOW_SOMA_BIFURCATIONS.h5");

            mutable_morph.write(tmpDirectory / "from_asc_ALLOW_SOMA_BIFURCATIONS.swc");
            morphio::mut::Morphology(tmpDirectory / "from_asc_ALLOW_SOMA_BIFURCATIONS.swc");
        }

        SECTION("testing ALLOW_ROOT_BIFURCATIONS option") {
            auto mutable_morph =
                morphio::mut::Morphology("data/edge_cases/root_bifurcation.asc",
                                         morphio::enums::Option::ALLOW_ROOT_BIFURCATIONS);
            REQUIRE_THROWS(
                mutable_morph.write(tmpDirectory / "from_asc_ALLOW_ROOT_BIFURCATIONS.asc"));
            REQUIRE_THROWS(
                mutable_morph.write(tmpDirectory / "from_asc_ALLOW_ROOT_BIFURCATIONS.h5"));
            REQUIRE_THROWS(
                mutable_morph.write(tmpDirectory / "from_asc_ALLOW_ROOT_BIFURCATIONS.swc"));
        }
    }

    SECTION("testing H5 to other formats") {
        SECTION("testing ALLOW_CUSTOM_ROOT_ID option") {
            // Can not be loaded with H5 format
        }

        SECTION("testing ALLOW_MULTIPLE_SOMATA option") {
            // Can not be loaded with H5 format
        }

        SECTION("testing ALLOW_SOMA_BIFURCATIONS option") {
            // Can not be loaded with H5 format
        }

        SECTION("testing ALLOW_ROOT_BIFURCATIONS option") {
            auto mutable_morph =
                morphio::mut::Morphology("data/edge_cases/root_bifurcation.h5",
                                         morphio::enums::Option::ALLOW_ROOT_BIFURCATIONS);
            REQUIRE_THROWS(
                mutable_morph.write(tmpDirectory / "from_asc_ALLOW_ROOT_BIFURCATIONS.asc"));
            REQUIRE_THROWS(
                mutable_morph.write(tmpDirectory / "from_asc_ALLOW_ROOT_BIFURCATIONS.h5"));
            REQUIRE_THROWS(
                mutable_morph.write(tmpDirectory / "from_asc_ALLOW_ROOT_BIFURCATIONS.swc"));
        }
    }
}
