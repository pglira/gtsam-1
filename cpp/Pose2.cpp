/**
 * @file  Pose2.cpp
 * @brief 2D Pose
 */

#include "Pose2.h"
#include "Lie-inl.h"

using namespace std;

namespace gtsam {

  /** Explicit instantiation of base class to export members */
  INSTANTIATE_LIE(Pose2);

	static const Matrix I3 = eye(3);

  /* ************************************************************************* */
  Matrix Pose2::matrix() const {
  	Matrix R = r_.matrix();
  	Matrix Z = zeros(1,2);
  	R = stack(2, &R, &Z);
  	Matrix T = Matrix_(3,1, t_.x(), t_.y(), 1.0);
  	return collect(2, &R, &T);
  }

  /* ************************************************************************* */
  void Pose2::print(const string& s) const {
    cout << s << "(" << t_.x() << ", " << t_.y() << ", " << r_.theta() << ")" << endl;
  }

  /* ************************************************************************* */
  bool Pose2::equals(const Pose2& q, double tol) const {
    return t_.equals(q.t_, tol) && r_.equals(q.r_, tol);
  }

  /* ************************************************************************* */
  Point2 transform_to(const Pose2& pose, const Point2& point, boost::optional<
			Matrix&> H1, boost::optional<Matrix&> H2) {
		const Rot2& R = pose.r();
		Point2 d = point - pose.t();
		Point2 q = R.unrotate(d);
		if (!H1 && !H2) return q;
		if (H1) *H1 = Matrix_(2, 3,
					-1.0, 0.0,  q.y(),
					0.0, -1.0, -q.x());
		if (H2) *H2 = R.transpose();
		return q;
	}

  Matrix Dtransform_to1(const Pose2& pose, const Point2& point) {
		Matrix H; transform_to(pose, point, H, boost::none); return H;
  }

  Matrix Dtransform_to2(const Pose2& pose, const Point2& point) {
		Matrix H; transform_to(pose, point, boost::none, H); return H;
  }

  /* ************************************************************************* */
  Pose2 between(const Pose2& p1, const Pose2& p2, boost::optional<Matrix&> H1,
			boost::optional<Matrix&> H2) {
  	// get cosines and sines from rotation matrices
  	const Rot2& R1 = p1.r(), R2 = p2.r();
  	double c1=R1.c(), s1=R1.s(), c2=R2.c(), s2=R2.s();

  	// Calculate delta rotation = between(R1,R2)
		double c = c1 * c2 + s1 * s2, s = -s1 * c2 + c1 * s2;
    Rot2 R(c,s);

  	// Calculate delta translation = unrotate(R1, dt);
		Point2 dt = p2.t() - p1.t();
		double x = dt.x(), y = dt.y();
		Point2 t(c1 * x + s1 * y, -s1 * x + c1 * y);

		// FD: I don't understand this code (a performance-driven transformation
		// from Richard's heavier code) but it works.
		if (H1) {
			double dt1 = -s2 * x + c2 * y;
			double dt2 = -c2 * x - s2 * y;
			H1->resize(3,3);
			double data[9] = {
				-c,  -s,  dt1,
				 s,  -c,  dt2,
			 0.0, 0.0, -1.0};
			 copy(data, data+9, H1->data().begin());
		}
		if (H2) *H2 = I3;

		return Pose2(R,t);
	}

  /* ************************************************************************* */
	Rot2 bearing(const Pose2& pose, const Point2& point) {
		Point2 d = transform_to(pose, point);
		return relativeBearing(d);
	}

	Rot2 bearing(const Pose2& pose, const Point2& point,
			boost::optional<Matrix&> H1, boost::optional<Matrix&> H2) {
		if (!H1 && !H2) return bearing(pose, point);
		Point2 d = transform_to(pose, point);
		Matrix D_result_d;
		Rot2 result = relativeBearing(d, D_result_d);
		if (H1) *H1 = D_result_d * Dtransform_to1(pose, point);
		if (H2) *H2 = D_result_d * Dtransform_to2(pose, point);
		return result;
	}

  /* ************************************************************************* */
	double range(const Pose2& pose, const Point2& point) {
		Point2 d = transform_to(pose, point);
		return d.norm();
	}

	double range(const Pose2& pose, const Point2& point,
			boost::optional<Matrix&> H1, boost::optional<Matrix&> H2) {
		if (!H1 && !H2) return range(pose, point);
		Point2 d = transform_to(pose, point);
		double x = d.x(), y = d.y(), d2 = x * x + y * y, n = sqrt(d2);
		Matrix D_result_d = Matrix_(1, 2, x / n, y / n);
		if (H1) *H1 = D_result_d * Dtransform_to1(pose, point);
		if (H2) *H2 = D_result_d * Dtransform_to2(pose, point);
		return n;
	}

  /* ************************************************************************* */
} // namespace gtsam
