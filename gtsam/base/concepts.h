/*
 * concepts.h
 *
 * @data Dec 4, 2014
 * @author Mike Bosse
 * @author Frank Dellaert
 */

#pragma once

//#include "manifold.h"
//#include "chart.h"
#include <gtsam/base/Matrix.h>
#include <gtsam/base/Testable.h>
#include <boost/concept_check.hpp>
#include <boost/concept/requires.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>

namespace gtsam {

namespace traits {

/**
 * @name Algebraic Structure Traits
 * @brief Associate a unique tag with each of the main GTSAM concepts
 */
//@{
template<typename T> struct structure_category;
//@}

/**
 * @name Algebraic Structure Tags
 * @brief Possible values for traits::structure_category<T>::type
 */
//@{
struct manifold_tag {};
struct group_tag {};
struct lie_group_tag: public manifold_tag, public group_tag {};
struct vector_space_tag: public lie_group_tag {};
//@}

}// namespace traits

namespace manifold {

/** @name Free functions any Manifold needs to define */
//@{
//@}

namespace traits {

/** @name Manifold Traits */
//@{
template<typename Manifold> struct dimension;
template<typename Manifold> struct TangentVector;
template<typename Manifold> struct DefaultChart;
//@}

}// \ namespace traits

/// Check invariants for Manifold type
template<typename T>
BOOST_CONCEPT_REQUIRES(((Testable<T>)),(bool)) //
check_invariants(const T& a, const T& b) {
  typedef typename traits::DefaultChart<T>::type Chart;
  return true;
}

} // \ namespace manifold

/**
 * Chart concept
 */
template<typename T>
class IsChart {
public:
  typedef typename T::ManifoldType ManifoldType;
  typedef typename manifold::traits::TangentVector<ManifoldType>::type V;

  BOOST_CONCEPT_USAGE(IsChart) {
    // make sure Derived methods in Chart are defined
    v = T::Local(p,q);
    q = T::Retract(p,v);
  }
private:
  ManifoldType p,q;
  V v;
};

/**
 * Manifold concept
 */
template<typename T>
class IsManifold {
public:
  typedef typename traits::structure_category<T>::type structure_category_tag;
  static const size_t dim = manifold::traits::dimension<T>::value;
  typedef typename manifold::traits::TangentVector<T>::type TangentVector;
  typedef typename manifold::traits::DefaultChart<T>::type DefaultChart;

  BOOST_CONCEPT_USAGE(IsManifold) {
    BOOST_STATIC_ASSERT_MSG(
        (boost::is_base_of<traits::manifold_tag, structure_category_tag>::value),
        "This type's structure_category trait does not assert it as a manifold (or derived)");
    BOOST_STATIC_ASSERT(TangentVector::SizeAtCompileTime == dim);
    BOOST_CONCEPT_ASSERT((IsChart<DefaultChart >));
  }
private:
  T p,q;
  TangentVector v;
};

namespace group {

/** @name Free functions any Group needs to define */
//@{
template<typename T> T compose(const T&g, const T& h);
template<typename T> T between(const T&g, const T& h);
template<typename T> T inverse(const T&g);
//@}

namespace traits {

/** @name Group Traits */
//@{
template<typename T> struct identity;
template<typename T> struct flavor;
//@}

/** @name Group Flavor Tags */
//@{
struct additive_tag {
};
struct multiplicative_tag {
};
//@}

}// \ namespace traits

/// Check invariants
template<typename T>
BOOST_CONCEPT_REQUIRES(((Testable<T>)),(bool)) //
check_invariants(const T& a, const T& b, double tol = 1e-9) {
  T e = traits::identity<T>::value;
  return compose(a, inverse(a)).equals(e, tol)
      && between(a, b).equals(compose(inverse(a), b), tol)
      && compose(a, between(a, b)).equals(b, tol);
}
} // \ namespace group

#define GTSAM_ADDITIVE_GROUP1(T1,A1,GROUP) \
namespace group { \
template<T1 A1> GROUP<A1> compose(const GROUP<A1> &g, const GROUP<A1> & h) { return g + h;} \
template<T1 A1> GROUP<A1> between(const GROUP<A1> &g, const GROUP<A1> & h) { return h - g;} \
template<T1 A1> GROUP<A1> inverse(const GROUP<A1> &g) { return -g;} \
namespace traits { \
template<T1 A1> struct identity<GROUP<A1> > { static const GROUP<A1> value; typedef GROUP<A1> value_type;};\
template<T1 A1> const GROUP<A1> identity<GROUP<A1> >::value = GROUP<A1>::Identity();\
template<T1 A1> struct flavor<GROUP<A1> > { typedef additive_tag type;};\
}}

#define GTSAM_MULTIPLICATIVE_GROUP2(T1,A1,T2,A2,GROUP) \
namespace group { \
template<T1 A1, T2 A2> GROUP<A1,A2> compose(const GROUP<A1,A2> &g, const GROUP<A1,A2> & h) { return g * h;} \
template<T1 A1, T2 A2> GROUP<A1,A2> between(const GROUP<A1,A2> &g, const GROUP<A1,A2> & h) { return g.inverse() * h;} \
template<T1 A1, T2 A2> GROUP<A1,A2> inverse(const GROUP<A1,A2> &g) { return g.inverse();} \
namespace traits { \
template<T1 A1, T2 A2> struct identity<GROUP<A1,A2> > { static const GROUP<A1,A2> value; typedef GROUP<A1,A2> value_type;};\
template<T1 A1, T2 A2> const GROUP<A1,A2> identity<GROUP<A1,A2> >::value = GROUP<A1,A2>::Identity();\
template<T1 A1, T2 A2> struct flavor<GROUP<A1,A2> > { typedef multiplicative_tag type;};\
}}

/**
 * Group Concept
 */
template<typename T>
class IsGroup {
public:

  typedef typename traits::structure_category<T>::type structure_category_tag;
  typedef typename group::traits::identity<T>::value_type identity_value_type;
  typedef typename group::traits::flavor<T>::type flavor_tag;

  void operator_usage(group::traits::multiplicative_tag) {
    g = g * h;
  }
  void operator_usage(group::traits::additive_tag) {
    g = g + h;
    g = h - g;
    g = -g;
  }

  BOOST_CONCEPT_USAGE(IsGroup) {
    BOOST_STATIC_ASSERT_MSG(
        (boost::is_base_of<traits::group_tag, structure_category_tag>::value),
        "This type's structure_category trait does not assert it as a group (or derived)");
    e = group::traits::identity<T>::value;
    g = group::compose(g, h);
    g = group::between(g, h);
    g = group::inverse(g);
    operator_usage(flavor);
  }

private:
  flavor_tag flavor;
  T e, g, h;
};

namespace lie_group {

/** @name Free functions any Group needs to define */
//@{
// TODO need Jacobians
//template<typename T> T compose(const T&g, const T& h);
//template<typename T> T between(const T&g, const T& h);
//template<typename T> T inverse(const T&g);
//@}

namespace traits {

/** @name Lie Group Traits */
//@{
//@}

}// \ namespace traits

/// Check invariants
//template<typename T>
//BOOST_CONCEPT_REQUIRES(((Testable<T>)),(bool)) check_invariants(const T& a,
//    const T& b) {
//  bool check_invariants(const V& a, const V& b) {
//    return equal(Chart::retract(a, b), a + b)
//        && equal(Chart::local(a, b), b - a);
//  }
//}
}// \ namespace lie_group

/**
 * Lie Group Concept
 */
template<typename T>
class IsLieGroup: public IsGroup<T>, public IsManifold<T> {
public:

  typedef typename traits::structure_category<T>::type structure_category_tag;

  BOOST_CONCEPT_USAGE(IsLieGroup) {
    BOOST_STATIC_ASSERT_MSG(
        (boost::is_base_of<traits::lie_group_tag, structure_category_tag>::value),
        "This type's trait does not assert it as a Lie group (or derived)");
    // TODO Check with Jacobian
//    using lie_group::compose;
//    using lie_group::between;
//    using lie_group::inverse;
//    g = compose(g, h);
//    g = between(g, h);
//    g = inverse(g);
  }
private:

  T g, h;
};

template<typename T>
class IsVectorSpace: public IsLieGroup<T> {
public:

  typedef typename traits::structure_category<T>::type structure_category_tag;

  BOOST_CONCEPT_USAGE(IsVectorSpace) {
    BOOST_STATIC_ASSERT_MSG(
        (boost::is_base_of<traits::vector_space_tag, structure_category_tag>::value),
        "This type's trait does not assert it as a vector space (or derived)");
    r = p + q;
    r = -p;
    r = p - q;
  }

private:
  T p, q, r;
};

} // namespace gtsam

