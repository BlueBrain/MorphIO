from pathlib import Path

import morphio
from morphio import Collection

DATA_DIR = Path(__file__).parent / "data"

def check_load_from_collection(collection):
    morphology_names = [
        "simple",
        "glia",
        "mitochondria",
        "endoplasmic-reticulum"
    ]
    for morph_name in morphology_names:
        morph = collection.load(morph_name)
        assert isinstance(morph, morphio.Morphology)

        morph = collection.load(morph_name, mutable=False)
        assert isinstance(morph, morphio.Morphology)

        morph = collection.load(morph_name, mutable=True)
        assert isinstance(morph, morphio.mut.Morphology)


def check_load_from_collection_with_context(collection_path):
    with morphio.Collection(collection_path) as collection:
        check_load_from_collection(collection)


def check_load_from_collection_without_context(collection_path):
    collection = morphio.Collection(collection_path)

    check_load_from_collection(collection)

    collection.close()


def test_directory_collection():
    collection_path = DATA_DIR / "h5/v1"
    check_load_from_collection_with_context(collection_path)
    check_load_from_collection_without_context(collection_path)

def test_merged_container_collection():
    collection_path = DATA_DIR / "h5/v1/merged.h5"
    check_load_from_collection_with_context(collection_path)
    check_load_from_collection_without_context(collection_path)

def test_unified_container_collection():
    collection_path = DATA_DIR / "h5/v1/unified.h5"
    check_load_from_collection_with_context(collection_path)
    check_load_from_collection_without_context(collection_path)
