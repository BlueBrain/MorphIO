# Copyright (c) 2013-2023, EPFL/Blue Brain Project
# SPDX-License-Identifier: Apache-2.0
from pathlib import Path

import numpy as np
import pytest

import morphio


DATA_DIR = Path(__file__).parent / "data"
COLLECTION_PATHS = [DATA_DIR / "h5/v1/merged.h5", DATA_DIR / "h5/v1"]


def available_morphologies():
    return [
        "simple",
        "glia",
        "mitochondria",
        "endoplasmic-reticulum",
        "simple-dendritric-spine"
    ]


def check_load_from_collection(collection):
    morphology_names = available_morphologies()

    for morph_name in morphology_names:
        morph = collection.load(morph_name)
        assert isinstance(morph, morphio.Morphology)

        morph = collection.load(morph_name, mutable=False)
        assert isinstance(morph, morphio.Morphology)

        morph = collection.load(morph_name, mutable=True)
        assert isinstance(morph, morphio.mut.Morphology)


@pytest.mark.parametrize("collection_path", COLLECTION_PATHS)
def test_load_from_collection_with_context(collection_path):
    with morphio.Collection(collection_path) as collection:
        check_load_from_collection(collection)


@pytest.mark.parametrize("collection_path", COLLECTION_PATHS)
def test_load_from_collection_without_context(collection_path):
    collection = morphio.Collection(collection_path)

    check_load_from_collection(collection)

    collection.close()


@pytest.mark.parametrize("collection_path", COLLECTION_PATHS)
def test_container_unordered(collection_path):
    with morphio.Collection(collection_path) as collection:
        morphology_names = available_morphologies()
        morphology_names = morphology_names[1:]

        loop_indices = []
        for k, morph in collection.load_unordered(morphology_names):
            loop_indices.append(k)

        np.testing.assert_array_equal(
            sorted(loop_indices),
            np.arange(len(morphology_names))
        )


@pytest.mark.parametrize("collection_path", COLLECTION_PATHS)
def test_container_unordered1(collection_path):
    with morphio.Collection(collection_path) as collection:
        morphology_names = available_morphologies()
        morphology_names = morphology_names[1:]

        loop_indices = collection.argsort(morphology_names)
        np.testing.assert_array_equal(
            sorted(loop_indices),
            np.arange(len(morphology_names))
        )
