from pathlib import Path
import pytest

import morphio
from morphio import Collection

DATA_DIR = Path(__file__).parent / "data"
COLLECTION_PATHS = [DATA_DIR / "h5/v1/merged.h5", DATA_DIR / "h5/v1"]

def check_load_from_collection(collection):
    morphology_names = [
        "simple",
        "glia",
        "mitochondria",
        "endoplasmic-reticulum",
        "simple-dendritric-spine"
    ]
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
