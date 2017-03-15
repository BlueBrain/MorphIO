#define BOOST_TEST_MODULE testMorpho
#define BOOST_TEST_MAIN

#include <iostream>
#include <sstream>
#include <memory>

#include <boost/test/unit_test.hpp>

#include <morpho/morpho_tree.hpp>



BOOST_AUTO_TEST_CASE( test_morpho_tree )
{
    using namespace morpho;

    morpho_tree tree;
    std::size_t id;

    BOOST_CHECK_EQUAL(tree.get_tree_size(), 0);

    id = tree.set_root(std::unique_ptr<branch>(new branch(branch_type::soma)));

    BOOST_CHECK_EQUAL(id, 0);
    BOOST_CHECK_EQUAL(tree.get_tree_size(), 1);

    BOOST_CHECK_EQUAL(tree.get_branch(0).get_points().size1(), 0);
    BOOST_CHECK_EQUAL(tree.get_branch(0).get_points().size2(), 0);
    BOOST_CHECK(tree.get_branch(0).get_type() == branch_type::soma);

    BOOST_CHECK_EQUAL(tree.get_branch(0).get_childrens().size(), 0);

    id = tree.add_child(0, std::unique_ptr<branch>(new branch(branch_type::dentrite_basal)));
    BOOST_CHECK_EQUAL(id, 1);
    BOOST_CHECK_EQUAL(tree.get_tree_size(), 2);
    BOOST_CHECK(tree.get_branch(1).get_type() == branch_type::dentrite_basal);

    BOOST_CHECK_EQUAL(tree.get_branch(0).get_childrens().size(), 1);
    BOOST_CHECK_EQUAL(tree.get_branch(0).get_childrens()[0], id);


    BOOST_CHECK_EQUAL(tree.get_branch(1).get_parent(), 0);
    
    // test swap method
    morpho_tree tree_copy;
    BOOST_CHECK_EQUAL(tree_copy.get_tree_size(), 0);
    
    tree_copy.swap(tree);
    BOOST_CHECK_EQUAL(tree_copy.get_tree_size(), 2); 
    BOOST_CHECK_EQUAL(tree.get_tree_size(), 0); 
}





BOOST_AUTO_TEST_CASE( test_spatial )
{
    using namespace morpho;

    morpho_tree tree;
    std::size_t id;

    BOOST_CHECK_EQUAL(tree.get_tree_size(), 0);

    id = tree.set_root(std::unique_ptr<branch>(new branch(branch_type::soma)));

    BOOST_CHECK_EQUAL(id, 0);
    BOOST_CHECK_EQUAL(tree.get_tree_size(), 1);

    BOOST_CHECK_EQUAL(tree.get_branch(0).get_points().size1(), 0);
    BOOST_CHECK_EQUAL(tree.get_branch(0).get_points().size2(), 0);
    BOOST_CHECK(tree.get_branch(0).get_type() == branch_type::soma);

    BOOST_CHECK_EQUAL(tree.get_branch(0).get_childrens().size(), 0);

    id = tree.add_child(0, std::unique_ptr<branch>(new branch(branch_type::dentrite_basal)));
    BOOST_CHECK_EQUAL(id, 1);
    BOOST_CHECK_EQUAL(tree.get_tree_size(), 2);
    BOOST_CHECK(tree.get_branch(1).get_type() == branch_type::dentrite_basal);

    BOOST_CHECK_EQUAL(tree.get_branch(0).get_childrens().size(), 1);
    BOOST_CHECK_EQUAL(tree.get_branch(0).get_childrens()[0], id);


    BOOST_CHECK_EQUAL(tree.get_branch(1).get_parent(), 0);

}

