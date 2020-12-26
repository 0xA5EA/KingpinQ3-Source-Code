/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if !defined(INCLUDED_MATH_MATRIX_H)
#define INCLUDED_MATH_MATRIX_H

/// \file
/// \brief Matrix data types and related operations.

#include "math/Vector3.h"
#include "math/Vector4.h"
#include "math/Plane3.h"

/// \brief A 4x4 matrix stored in double-precision floating-point.
class Matrix4
{
  double m_elements[16];
public:

	// Default constructor
	Matrix4() {}
	
	/* NAMED CONSTRUCTORS FOR SPECIFIC MATRICES */
	
	/* Get a new identity matrix */

	static Matrix4 getIdentity() {
		return Matrix4(1, 0, 0, 0,
						0, 1, 0, 0,
						0, 0, 1, 0,
						0, 0, 0, 1);
	}
	
	/** Get a matrix representing the given 3D translation.
	 * 
	 * @param translation
	 * Vector3 representing the translation in 3D space.
	 */
	 
	static Matrix4 getTranslation(const Vector3& translation) {
		return Matrix4(1, 0, 0, 0,
						0, 1, 0, 0,
						0, 0, 1, 0,
						translation.x(), translation.y(), translation.z(), 1);
	}
	
	/* greebo: Returns the rotation matrix defined by two three-component 
	 * vectors.
	 * The rotational axis is defined by the normalised cross product of those 
	 * two vectors, the angle can be retrieved from the dot product.
	 */
	static Matrix4 getRotation(const Vector3& a, const Vector3& b);

	// Custom constructor

  Matrix4(double xx_, double xy_, double xz_, double xw_,
    double yx_, double yy_, double yz_, double yw_,
    double zx_, double zy_, double zz_, double zw_,
    double tx_, double ty_, double tz_, double tw_)
  {
    xx() = xx_;
    xy() = xy_;
    xz() = xz_;
    xw() = xw_;
    yx() = yx_;
    yy() = yy_;
    yz() = yz_;
    yw() = yw_;
    zx() = zx_;
    zy() = zy_;
    zz() = zz_;
    zw() = zw_;
    tx() = tx_;
    ty() = ty_;
    tz() = tz_;
    tw() = tw_;
  }

  double& xx()
  {
    return m_elements[0];
  }
  const double& xx() const
  {
    return m_elements[0];
  }
  double& xy()
  {
    return m_elements[1];
  }
  const double& xy() const
  {
    return m_elements[1];
  }
  double& xz()
  {
    return m_elements[2];
  }
  const double& xz() const
  {
    return m_elements[2];
  }
  double& xw()
  {
    return m_elements[3];
  }
  const double& xw() const
  {
    return m_elements[3];
  }
  double& yx()
  {
    return m_elements[4];
  }
  const double& yx() const
  {
    return m_elements[4];
  }
  double& yy()
  {
    return m_elements[5];
  }
  const double& yy() const
  {
    return m_elements[5];
  }
  double& yz()
  {
    return m_elements[6];
  }
  const double& yz() const
  {
    return m_elements[6];
  }
  double& yw()
  {
    return m_elements[7];
  }
  const double& yw() const
  {
    return m_elements[7];
  }
  double& zx()
  {
    return m_elements[8];
  }
  const double& zx() const
  {
    return m_elements[8];
  }
  double& zy()
  {
    return m_elements[9];
  }
  const double& zy() const
  {
    return m_elements[9];
  }
  double& zz()
  {
    return m_elements[10];
  }
  const double& zz() const
  {
    return m_elements[10];
  }
  double& zw()
  {
    return m_elements[11];
  }
  const double& zw() const
  {
    return m_elements[11];
  }
  double& tx()
  {
    return m_elements[12];
  }
  const double& tx() const
  {
    return m_elements[12];
  }
  double& ty()
  {
    return m_elements[13];
  }
  const double& ty() const
  {
    return m_elements[13];
  }
  double& tz()
  {
    return m_elements[14];
  }
  const double& tz() const
  {
    return m_elements[14];
  }
  double& tw()
  {
    return m_elements[15];
  }
  const double& tw() const
  {
    return m_elements[15];
  }

  Vector4& x()
  {
    return reinterpret_cast<Vector4&>(xx());
  }
  const Vector4& x() const
  {
    return reinterpret_cast<const Vector4&>(xx());
  }
  Vector4& y()
  {
    return reinterpret_cast<Vector4&>(yx());
  }
  const Vector4& y() const
  {
    return reinterpret_cast<const Vector4&>(yx());
  }
  Vector4& z()
  {
    return reinterpret_cast<Vector4&>(zx());
  }
  const Vector4& z() const
  {
    return reinterpret_cast<const Vector4&>(zx());
  }
  Vector4& t()
  {
    return reinterpret_cast<Vector4&>(tx());
  }
  const Vector4& t() const
  {
    return reinterpret_cast<const Vector4&>(tx());
  }

  const double& index(std::size_t i) const
  {
    return m_elements[i];
  }
  double& index(std::size_t i)
  {
    return m_elements[i];
  }
  const double& index(std::size_t r, std::size_t c) const
  {
    return m_elements[(r << 2) + c];
  }
  double& index(std::size_t r, std::size_t c)
  {
    return m_elements[(r << 2) + c];
  }

	/** Cast to double* for use with GL functions that accept a double
	 * array, also provides operator[].
	 */
	operator double* () {
		return m_elements;	
	}

	/** Cast to const double* to provide operator[] for const objects.
	 */
	operator const double* () const {
		return m_elements;
	}

	// Transposes the matrix <self>
	void transpose() {
		std::swap(m_elements[1], m_elements[4]); // xy <=> yx
		std::swap(m_elements[2], m_elements[8]); // xz <=> zx
		std::swap(m_elements[3], m_elements[12]); // xw <=> tx
		std::swap(m_elements[6], m_elements[9]); // yz <=> zy
		std::swap(m_elements[7], m_elements[13]); // yw <=> ty
		std::swap(m_elements[11], m_elements[14]); // zw <=> tz
	}
	
	// Returns the transposed matrix of <self>
	Matrix4 getTransposed() const {
		return Matrix4(
			xx(), yx(), zx(), tx(),
			xy(), yy(), zy(), ty(),
			xz(), yz(), zz(), tz(),
			xw(), yw(), zw(), tw()
		);
	}

	/** Use this matrix to transform the provided vector and return a new
	 * vector containing the result.
	 * 
	 * @param vector4
	 * The 4-element vector to transform.
	 */
	Vector4 transform(const Vector4& vector4) const {
		return Vector4(
		    m_elements[0]  * vector4[0] + m_elements[4]  * vector4[1] + m_elements[8]  * vector4[2] + m_elements[12] * vector4[3],
		    m_elements[1]  * vector4[0] + m_elements[5]  * vector4[1] + m_elements[9]  * vector4[2] + m_elements[13] * vector4[3],
		    m_elements[2]  * vector4[0] + m_elements[6]  * vector4[1] + m_elements[10] * vector4[2] + m_elements[14] * vector4[3],
		    m_elements[3]  * vector4[0] + m_elements[7]  * vector4[1] + m_elements[11] * vector4[2] + m_elements[15] * vector4[3]
		);
	}
	
	/** Use this matrix to transform the provided 3-element vector,
	 * automatically converting the vector to a 4-element homogeneous
	 * vector with w=1.
	 * 
	 * @param vector3
	 * The Vector3 to transform.
	 * 
	 * @returns
	 * A 4-element vector containing the result.
	 */
	Vector4 transform(const Vector3& vector3) const {
		return transform(Vector4(vector3, 1));
	}
	
	/** Use this matrix to transform the provided plane
	 * 
	 * @param plane: The Plane to transform.
	 * 
	 * @returns: the transformed plane.
	 */
	Plane3 transform(const Plane3& plane) const {
		Plane3 transformed;
		transformed.a = m_elements[0] * plane.a + m_elements[4] * plane.b + m_elements[8] * plane.c;
		transformed.b = m_elements[1] * plane.a + m_elements[5] * plane.b + m_elements[9] * plane.c;
		transformed.c = m_elements[2] * plane.a + m_elements[6] * plane.b + m_elements[10] * plane.c;
		transformed.d = -(	(-plane.d * transformed.a + m_elements[12]) * transformed.a + 
							(-plane.d * transformed.b + m_elements[13]) * transformed.b + 
							(-plane.d * transformed.c + m_elements[14]) * transformed.c);
		return transformed;
	}
	
	Plane3 inverseTransform(const Plane3& plane) const {
		return Plane3(
			m_elements[ 0] * plane.a + m_elements[ 1] * plane.b + m_elements[ 2] * plane.c + m_elements[ 3] * plane.d,
			m_elements[ 4] * plane.a + m_elements[ 5] * plane.b + m_elements[ 6] * plane.c + m_elements[ 7] * plane.d,
			m_elements[ 8] * plane.a + m_elements[ 9] * plane.b + m_elements[10] * plane.c + m_elements[11] * plane.d,
			m_elements[12] * plane.a + m_elements[13] * plane.b + m_elements[14] * plane.c + m_elements[15] * plane.d
		);
	}
	
};

/// \brief The 4x4 identity matrix.
const Matrix4 g_matrix4_identity(
  1, 0, 0, 0,
  0, 1, 0, 0,
  0, 0, 1, 0,
  0, 0, 0, 1
);


/// \brief Returns true if \p self and \p other are exactly element-wise equal.
inline bool operator==(const Matrix4& self, const Matrix4& other)
{
  return self.xx() == other.xx() && self.xy() == other.xy() && self.xz() == other.xz() && self.xw() == other.xw()
    && self.yx() == other.yx() && self.yy() == other.yy() && self.yz() == other.yz() && self.yw() == other.yw()
    && self.zx() == other.zx() && self.zy() == other.zy() && self.zz() == other.zz() && self.zw() == other.zw()
    && self.tx() == other.tx() && self.ty() == other.ty() && self.tz() == other.tz() && self.tw() == other.tw();
}

/// \brief Returns true if \p self and \p other are exactly element-wise equal.
inline bool matrix4_equal(const Matrix4& self, const Matrix4& other)
{
  return self == other;
}

/// \brief Returns true if \p self and \p other are element-wise equal within \p epsilon.
inline bool matrix4_equal_epsilon(const Matrix4& self, const Matrix4& other, double epsilon)
{
  return float_equal_epsilon(self.xx(), other.xx(), epsilon)
    && float_equal_epsilon(self.xy(), other.xy(), epsilon)
    && float_equal_epsilon(self.xz(), other.xz(), epsilon)
    && float_equal_epsilon(self.xw(), other.xw(), epsilon)
    && float_equal_epsilon(self.yx(), other.yx(), epsilon)
    && float_equal_epsilon(self.yy(), other.yy(), epsilon)
    && float_equal_epsilon(self.yz(), other.yz(), epsilon)
    && float_equal_epsilon(self.yw(), other.yw(), epsilon)
    && float_equal_epsilon(self.zx(), other.zx(), epsilon)
    && float_equal_epsilon(self.zy(), other.zy(), epsilon)
    && float_equal_epsilon(self.zz(), other.zz(), epsilon)
    && float_equal_epsilon(self.zw(), other.zw(), epsilon)
    && float_equal_epsilon(self.tx(), other.tx(), epsilon)
    && float_equal_epsilon(self.ty(), other.ty(), epsilon)
    && float_equal_epsilon(self.tz(), other.tz(), epsilon)
    && float_equal_epsilon(self.tw(), other.tw(), epsilon);
}

/// \brief Returns true if \p self and \p other are exactly element-wise equal.
/// \p self and \p other must be affine.
inline bool matrix4_affine_equal(const Matrix4& self, const Matrix4& other)
{
  return self[0] == other[0]
    && self[1] == other[1]
    && self[2] == other[2]
    && self[4] == other[4]
    && self[5] == other[5]
    && self[6] == other[6]
    && self[8] == other[8]
    && self[9] == other[9]
    && self[10] == other[10]
    && self[12] == other[12]
    && self[13] == other[13]
    && self[14] == other[14];
}

enum Matrix4Handedness
{
  MATRIX4_RIGHTHANDED = 0,
  MATRIX4_LEFTHANDED = 1,
};

/// \brief Returns MATRIX4_RIGHTHANDED if \p self is right-handed, else returns MATRIX4_LEFTHANDED.
inline Matrix4Handedness matrix4_handedness(const Matrix4& self)
{
  return (
    self.x().getVector3().crossProduct(self.y().getVector3()).dot(self.z().getVector3()) < 0.0
  ) ? MATRIX4_LEFTHANDED : MATRIX4_RIGHTHANDED;
}





/// \brief Returns \p self post-multiplied by \p other.
inline Matrix4 matrix4_multiplied_by_matrix4(const Matrix4& self, const Matrix4& other)
{
  return Matrix4(
    other[0] * self[0] + other[1] * self[4] + other[2] * self[8] + other[3] * self[12],
    other[0] * self[1] + other[1] * self[5] + other[2] * self[9] + other[3] * self[13],
    other[0] * self[2] + other[1] * self[6] + other[2] * self[10]+ other[3] * self[14],
    other[0] * self[3] + other[1] * self[7] + other[2] * self[11]+ other[3] * self[15],
    other[4] * self[0] + other[5] * self[4] + other[6] * self[8] + other[7] * self[12],
    other[4] * self[1] + other[5] * self[5] + other[6] * self[9] + other[7] * self[13],
    other[4] * self[2] + other[5] * self[6] + other[6] * self[10]+ other[7] * self[14],
    other[4] * self[3] + other[5] * self[7] + other[6] * self[11]+ other[7] * self[15],
    other[8] * self[0] + other[9] * self[4] + other[10]* self[8] + other[11]* self[12],
    other[8] * self[1] + other[9] * self[5] + other[10]* self[9] + other[11]* self[13],
    other[8] * self[2] + other[9] * self[6] + other[10]* self[10]+ other[11]* self[14],
    other[8] * self[3] + other[9] * self[7] + other[10]* self[11]+ other[11]* self[15],
    other[12]* self[0] + other[13]* self[4] + other[14]* self[8] + other[15]* self[12],
    other[12]* self[1] + other[13]* self[5] + other[14]* self[9] + other[15]* self[13],
    other[12]* self[2] + other[13]* self[6] + other[14]* self[10]+ other[15]* self[14],
    other[12]* self[3] + other[13]* self[7] + other[14]* self[11]+ other[15]* self[15]
  );
}

/// \brief Post-multiplies \p self by \p other in-place.
inline void matrix4_multiply_by_matrix4(Matrix4& self, const Matrix4& other)
{
  self = matrix4_multiplied_by_matrix4(self, other);
}


/// \brief Returns \p self pre-multiplied by \p other.
inline Matrix4 matrix4_premultiplied_by_matrix4(const Matrix4& self, const Matrix4& other)
{
#if 1
  return matrix4_multiplied_by_matrix4(other, self);
#else
  return Matrix4(
    self[0] * other[0] + self[1] * other[4] + self[2] * other[8] + self[3] * other[12],
    self[0] * other[1] + self[1] * other[5] + self[2] * other[9] + self[3] * other[13],
    self[0] * other[2] + self[1] * other[6] + self[2] * other[10]+ self[3] * other[14],
    self[0] * other[3] + self[1] * other[7] + self[2] * other[11]+ self[3] * other[15],
    self[4] * other[0] + self[5] * other[4] + self[6] * other[8] + self[7] * other[12],
    self[4] * other[1] + self[5] * other[5] + self[6] * other[9] + self[7] * other[13],
    self[4] * other[2] + self[5] * other[6] + self[6] * other[10]+ self[7] * other[14],
    self[4] * other[3] + self[5] * other[7] + self[6] * other[11]+ self[7] * other[15],
    self[8] * other[0] + self[9] * other[4] + self[10]* other[8] + self[11]* other[12],
    self[8] * other[1] + self[9] * other[5] + self[10]* other[9] + self[11]* other[13],
    self[8] * other[2] + self[9] * other[6] + self[10]* other[10]+ self[11]* other[14],
    self[8] * other[3] + self[9] * other[7] + self[10]* other[11]+ self[11]* other[15],
    self[12]* other[0] + self[13]* other[4] + self[14]* other[8] + self[15]* other[12],
    self[12]* other[1] + self[13]* other[5] + self[14]* other[9] + self[15]* other[13],
    self[12]* other[2] + self[13]* other[6] + self[14]* other[10]+ self[15]* other[14],
    self[12]* other[3] + self[13]* other[7] + self[14]* other[11]+ self[15]* other[15]
  );
#endif
}

/// \brief Pre-multiplies \p self by \p other in-place.
inline void matrix4_premultiply_by_matrix4(Matrix4& self, const Matrix4& other)
{
  self = matrix4_premultiplied_by_matrix4(self, other);
}

/// \brief returns true if \p transform is affine.
inline bool matrix4_is_affine(const Matrix4& transform)
{
  return transform[3] == 0 && transform[7] == 0 && transform[11] == 0 && transform[15] == 1;
}

/// \brief Returns \p self post-multiplied by \p other.
/// \p self and \p other must be affine.
inline Matrix4 matrix4_affine_multiplied_by_matrix4(const Matrix4& self, const Matrix4& other)
{
  return Matrix4(
    other[0] * self[0] + other[1] * self[4] + other[2] * self[8],
    other[0] * self[1] + other[1] * self[5] + other[2] * self[9],
    other[0] * self[2] + other[1] * self[6] + other[2] * self[10],
    0,
    other[4] * self[0] + other[5] * self[4] + other[6] * self[8],
    other[4] * self[1] + other[5] * self[5] + other[6] * self[9],
    other[4] * self[2] + other[5] * self[6] + other[6] * self[10],
    0,
     other[8] * self[0] + other[9] * self[4] + other[10]* self[8],
    other[8] * self[1] + other[9] * self[5] + other[10]* self[9],
    other[8] * self[2] + other[9] * self[6] + other[10]* self[10],
    0,
    other[12]* self[0] + other[13]* self[4] + other[14]* self[8] + self[12],
    other[12]* self[1] + other[13]* self[5] + other[14]* self[9] + self[13],
    other[12]* self[2] + other[13]* self[6] + other[14]* self[10]+ self[14],
    1
  );
}

/// \brief Post-multiplies \p self by \p other in-place.
/// \p self and \p other must be affine.
inline void matrix4_affine_multiply_by_matrix4(Matrix4& self, const Matrix4& other)
{
  self = matrix4_affine_multiplied_by_matrix4(self, other);
}

/// \brief Returns \p self pre-multiplied by \p other.
/// \p self and \p other must be affine.
inline Matrix4 matrix4_affine_premultiplied_by_matrix4(const Matrix4& self, const Matrix4& other)
{
#if 1
  return matrix4_affine_multiplied_by_matrix4(other, self);
#else
  return Matrix4(
    self[0] * other[0] + self[1] * other[4] + self[2] * other[8],
    self[0] * other[1] + self[1] * other[5] + self[2] * other[9],
    self[0] * other[2] + self[1] * other[6] + self[2] * other[10],
    0,
    self[4] * other[0] + self[5] * other[4] + self[6] * other[8],
    self[4] * other[1] + self[5] * other[5] + self[6] * other[9],
    self[4] * other[2] + self[5] * other[6] + self[6] * other[10],
    0,
    self[8] * other[0] + self[9] * other[4] + self[10]* other[8],
    self[8] * other[1] + self[9] * other[5] + self[10]* other[9],
    self[8] * other[2] + self[9] * other[6] + self[10]* other[10],
    0,
    self[12]* other[0] + self[13]* other[4] + self[14]* other[8] + other[12],
    self[12]* other[1] + self[13]* other[5] + self[14]* other[9] + other[13],
    self[12]* other[2] + self[13]* other[6] + self[14]* other[10]+ other[14],
    1
    )
  );
#endif
}

/// \brief Pre-multiplies \p self by \p other in-place.
/// \p self and \p other must be affine.
inline void matrix4_affine_premultiply_by_matrix4(Matrix4& self, const Matrix4& other)
{
  self = matrix4_affine_premultiplied_by_matrix4(self, other);
}

/// \brief Returns \p point transformed by \p self.
template<typename Element>
inline BasicVector3<Element> matrix4_transformed_point(const Matrix4& self, const BasicVector3<Element>& point)
{
  return BasicVector3<Element>(
    static_cast<Element>(self[0]  * point[0] + self[4]  * point[1] + self[8]  * point[2] + self[12]),
    static_cast<Element>(self[1]  * point[0] + self[5]  * point[1] + self[9]  * point[2] + self[13]),
    static_cast<Element>(self[2]  * point[0] + self[6]  * point[1] + self[10] * point[2] + self[14])
  );
}

/// \brief Transforms \p point by \p self in-place.
template<typename Element>
inline void matrix4_transform_point(const Matrix4& self, BasicVector3<Element>& point)
{
  point = matrix4_transformed_point(self, point);
}

/// \brief Returns \p direction transformed by \p self.
template<typename Element>
inline BasicVector3<Element> matrix4_transformed_direction(const Matrix4& self, const BasicVector3<Element>& direction)
{
  return BasicVector3<Element>(
    static_cast<Element>(self[0]  * direction[0] + self[4]  * direction[1] + self[8]  * direction[2]),
    static_cast<Element>(self[1]  * direction[0] + self[5]  * direction[1] + self[9]  * direction[2]),
    static_cast<Element>(self[2]  * direction[0] + self[6]  * direction[1] + self[10] * direction[2])
  );
}

/// \brief Transforms \p direction by \p self in-place.
template<typename Element>
inline void matrix4_transform_direction(const Matrix4& self, BasicVector3<Element>& normal)
{
  normal = matrix4_transformed_direction(self, normal);
}

/// \brief Returns \p vector4 transformed by \p self.
inline Vector4 matrix4_transformed_vector4(const Matrix4& self, const Vector4& vector4)
{
  return Vector4(
    self[0]  * vector4[0] + self[4]  * vector4[1] + self[8]  * vector4[2] + self[12] * vector4[3],
    self[1]  * vector4[0] + self[5]  * vector4[1] + self[9]  * vector4[2] + self[13] * vector4[3],
    self[2]  * vector4[0] + self[6]  * vector4[1] + self[10] * vector4[2] + self[14] * vector4[3],
    self[3]  * vector4[0] + self[7]  * vector4[1] + self[11] * vector4[2] + self[15] * vector4[3]
  );
}

/// \brief Transforms \p vector4 by \p self in-place.
inline void matrix4_transform_vector4(const Matrix4& self, Vector4& vector4)
{
  vector4 = matrix4_transformed_vector4(self, vector4);
}


/// \brief Inverts an affine transform in-place.
/// Adapted from Graphics Gems 2.
inline Matrix4 matrix4_affine_inverse(const Matrix4& self)
{
  Matrix4 result;
  
  // determinant of rotation submatrix
  double det
    = self[0] * ( self[5]*self[10] - self[9]*self[6] )
    - self[1] * ( self[4]*self[10] - self[8]*self[6] )
    + self[2] * ( self[4]*self[9] - self[8]*self[5] );

  // throw exception here if (det*det < 1e-25)
  
  // invert rotation submatrix
  det = 1.0 / det;

  result[0] = (  (self[5]*self[10]- self[6]*self[9] )*det);
  result[1] = (- (self[1]*self[10]- self[2]*self[9] )*det);
  result[2] = (  (self[1]*self[6] - self[2]*self[5] )*det);
  result[3] = 0;
  result[4] = (- (self[4]*self[10]- self[6]*self[8] )*det);
  result[5] = (  (self[0]*self[10]- self[2]*self[8] )*det);
  result[6] = (- (self[0]*self[6] - self[2]*self[4] )*det);
  result[7] = 0;
  result[8] = (  (self[4]*self[9] - self[5]*self[8] )*det);
  result[9] = (- (self[0]*self[9] - self[1]*self[8] )*det);
  result[10]= (  (self[0]*self[5] - self[1]*self[4] )*det);
  result[11] = 0;

  // multiply translation part by rotation
  result[12] = - (self[12] * result[0] +
    self[13] * result[4] +
    self[14] * result[8]);
  result[13] = - (self[12] * result[1] +
    self[13] * result[5] +
    self[14] * result[9]);
  result[14] = - (self[12] * result[2] +
    self[13] * result[6] +
    self[14] * result[10]);
  result[15] = 1;

  return result;
}

inline void matrix4_affine_invert(Matrix4& self)
{
  self = matrix4_affine_inverse(self);
}

/// \brief A compile-time-constant integer.
template<int VALUE_>
struct IntegralConstant
{
  enum unnamed_{ VALUE = VALUE_ };
};

/// \brief A compile-time-constant row/column index into a 4x4 matrix.
template<typename Row, typename Col>
class Matrix4Index
{
public:
  typedef IntegralConstant<Row::VALUE> r;
  typedef IntegralConstant<Col::VALUE> c;
  typedef IntegralConstant<(r::VALUE * 4) + c::VALUE> i;
};

/// \brief A functor which returns the cofactor of a 3x3 submatrix obtained by ignoring a given row and column of a 4x4 matrix.
/// \param Row Defines the compile-time-constant integers x, y and z with values corresponding to the indices of the three rows to use.
/// \param Col Defines the compile-time-constant integers x, y and z with values corresponding to the indices of the three columns to use.
template<typename Row, typename Col>
class Matrix4Cofactor
{
public:
  typedef typename Matrix4Index<typename Row::x, typename Col::x>::i xx;
  typedef typename Matrix4Index<typename Row::x, typename Col::y>::i xy;
  typedef typename Matrix4Index<typename Row::x, typename Col::z>::i xz;
  typedef typename Matrix4Index<typename Row::y, typename Col::x>::i yx;
  typedef typename Matrix4Index<typename Row::y, typename Col::y>::i yy;
  typedef typename Matrix4Index<typename Row::y, typename Col::z>::i yz;
  typedef typename Matrix4Index<typename Row::z, typename Col::x>::i zx;
  typedef typename Matrix4Index<typename Row::z, typename Col::y>::i zy;
  typedef typename Matrix4Index<typename Row::z, typename Col::z>::i zz;
  static double apply(const Matrix4& self)
  {
    return self[xx::VALUE] * ( self[yy::VALUE]*self[zz::VALUE] - self[zy::VALUE]*self[yz::VALUE] )
      - self[xy::VALUE] * ( self[yx::VALUE]*self[zz::VALUE] - self[zx::VALUE]*self[yz::VALUE] )
      + self[xz::VALUE] * ( self[yx::VALUE]*self[zy::VALUE] - self[zx::VALUE]*self[yy::VALUE] );
  }
};

/// \brief The cofactor element indices for a 4x4 matrix row or column.
/// \param Element The index of the element to ignore.
template<int Element>
class Cofactor4
{
public:
  typedef IntegralConstant<(Element <= 0) ? 1 : 0> x;
  typedef IntegralConstant<(Element <= 1) ? 2 : 1> y;
  typedef IntegralConstant<(Element <= 2) ? 3 : 2> z;
};

/// \brief Returns the determinant of \p self.
inline double matrix4_determinant(const Matrix4& self)
{
  return self.xx() * Matrix4Cofactor< Cofactor4<0>, Cofactor4<0> >::apply(self)
    - self.xy() * Matrix4Cofactor< Cofactor4<0>, Cofactor4<1> >::apply(self)
    + self.xz() * Matrix4Cofactor< Cofactor4<0>, Cofactor4<2> >::apply(self)
    - self.xw() * Matrix4Cofactor< Cofactor4<0>, Cofactor4<3> >::apply(self);
}

/// \brief Returns the inverse of \p self using the Adjoint method.
/// \todo Throw an exception if the determinant is zero.
inline Matrix4 matrix4_full_inverse(const Matrix4& self)
{
  double determinant = 1.0 / matrix4_determinant(self);

  return Matrix4(
    static_cast<double>( Matrix4Cofactor< Cofactor4<0>, Cofactor4<0> >::apply(self) * determinant),
    static_cast<double>(-Matrix4Cofactor< Cofactor4<1>, Cofactor4<0> >::apply(self) * determinant),
    static_cast<double>( Matrix4Cofactor< Cofactor4<2>, Cofactor4<0> >::apply(self) * determinant),
    static_cast<double>(-Matrix4Cofactor< Cofactor4<3>, Cofactor4<0> >::apply(self) * determinant),
    static_cast<double>(-Matrix4Cofactor< Cofactor4<0>, Cofactor4<1> >::apply(self) * determinant),
    static_cast<double>( Matrix4Cofactor< Cofactor4<1>, Cofactor4<1> >::apply(self) * determinant),
    static_cast<double>(-Matrix4Cofactor< Cofactor4<2>, Cofactor4<1> >::apply(self) * determinant),
    static_cast<double>( Matrix4Cofactor< Cofactor4<3>, Cofactor4<1> >::apply(self) * determinant),
    static_cast<double>( Matrix4Cofactor< Cofactor4<0>, Cofactor4<2> >::apply(self) * determinant),
    static_cast<double>(-Matrix4Cofactor< Cofactor4<1>, Cofactor4<2> >::apply(self) * determinant),
    static_cast<double>( Matrix4Cofactor< Cofactor4<2>, Cofactor4<2> >::apply(self) * determinant),
    static_cast<double>(-Matrix4Cofactor< Cofactor4<3>, Cofactor4<2> >::apply(self) * determinant),
    static_cast<double>(-Matrix4Cofactor< Cofactor4<0>, Cofactor4<3> >::apply(self) * determinant),
    static_cast<double>( Matrix4Cofactor< Cofactor4<1>, Cofactor4<3> >::apply(self) * determinant),
    static_cast<double>(-Matrix4Cofactor< Cofactor4<2>, Cofactor4<3> >::apply(self) * determinant),
    static_cast<double>( Matrix4Cofactor< Cofactor4<3>, Cofactor4<3> >::apply(self) * determinant)
  );
}

/// \brief Inverts \p self in-place using the Adjoint method.
inline void matrix4_full_invert(Matrix4& self)
{
  self = matrix4_full_inverse(self);
}


/// \brief Returns the translation part of \p self.
inline Vector3 matrix4_get_translation_vec3(const Matrix4& self)
{
  return self.t().getVector3();
}

/// \brief Concatenates \p self with \p translation.
/// The concatenated \p translation occurs before \p self.
inline void matrix4_translate_by_vec3(Matrix4& self, const Vector3& translation)
{
  matrix4_multiply_by_matrix4(self, Matrix4::getTranslation(translation));
}

/// \brief Returns \p self Concatenated with \p translation.
/// The concatenated translation occurs before \p self.
inline Matrix4 matrix4_translated_by_vec3(const Matrix4& self, const Vector3& translation)
{
  return matrix4_multiplied_by_matrix4(self, Matrix4::getTranslation(translation));
}


#include "math/pi.h"

/// \brief Returns \p angle modulated by the range [0, 360).
/// \p angle must be in the range [-360, 360).
inline double angle_modulate_degrees_range(double angle)
{
  return static_cast<double>(float_mod_range(angle, 360.0));
}

/// \brief Returns \p euler angles converted from radians to degrees.
inline Vector3 euler_radians_to_degrees(const Vector3& euler)
{
  return Vector3(
    static_cast<double>(radians_to_degrees(euler.x())),
    static_cast<double>(radians_to_degrees(euler.y())),
    static_cast<double>(radians_to_degrees(euler.z()))
  );
}

/// \brief Returns \p euler angles converted from degrees to radians.
inline Vector3 euler_degrees_to_radians(const Vector3& euler)
{
  return Vector3(
    static_cast<double>(degrees_to_radians(euler.x())),
    static_cast<double>(degrees_to_radians(euler.y())),
    static_cast<double>(degrees_to_radians(euler.z()))
  );
}



/// \brief Constructs a pure-rotation matrix about the x axis from sin \p s and cosine \p c of an angle.
inline Matrix4 matrix4_rotation_for_sincos_x(double s, double c)
{
  return Matrix4(
    1, 0, 0, 0,
    0, c, s, 0,
    0,-s, c, 0,
    0, 0, 0, 1
  );
}

/// \brief Constructs a pure-rotation matrix about the x axis from an angle in radians.
inline Matrix4 matrix4_rotation_for_x(double x)
{
  return matrix4_rotation_for_sincos_x(sin(x), cos(x));
}

/// \brief Constructs a pure-rotation matrix about the x axis from an angle in degrees.
inline Matrix4 matrix4_rotation_for_x_degrees(double x)
{
  return matrix4_rotation_for_x(degrees_to_radians(x));
}

/// \brief Constructs a pure-rotation matrix about the y axis from sin \p s and cosine \p c of an angle.
inline Matrix4 matrix4_rotation_for_sincos_y(double s, double c)
{
  return Matrix4(
    c, 0,-s, 0,
    0, 1, 0, 0,
    s, 0, c, 0,
    0, 0, 0, 1
  );
}

/// \brief Constructs a pure-rotation matrix about the y axis from an angle in radians.
inline Matrix4 matrix4_rotation_for_y(double y)
{
  return matrix4_rotation_for_sincos_y(sin(y), cos(y));
}

/// \brief Constructs a pure-rotation matrix about the y axis from an angle in degrees.
inline Matrix4 matrix4_rotation_for_y_degrees(double y)
{
  return matrix4_rotation_for_y(degrees_to_radians(y));
}

/// \brief Constructs a pure-rotation matrix about the z axis from sin \p s and cosine \p c of an angle.
inline Matrix4 matrix4_rotation_for_sincos_z(double s, double c)
{
  return Matrix4(
    c, s, 0, 0,
   -s, c, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  );
}

/// \brief Constructs a pure-rotation matrix about the z axis from an angle in radians.
inline Matrix4 matrix4_rotation_for_z(double z)
{
  return matrix4_rotation_for_sincos_z(sin(z), cos(z));
}

/// \brief Constructs a pure-rotation matrix about the z axis from an angle in degrees.
inline Matrix4 matrix4_rotation_for_z_degrees(double z)
{
  return matrix4_rotation_for_z(degrees_to_radians(z));
}

/// \brief Constructs a pure-rotation matrix from a set of euler angles (radians) in the order (x, y, z).
/*! \verbatim
clockwise rotation around X, Y, Z, facing along axis
 1  0   0    cy 0 -sy   cz  sz 0
 0  cx  sx   0  1  0   -sz  cz 0
 0 -sx  cx   sy 0  cy   0   0  1

rows of Z by cols of Y
 cy*cz -sy*cz+sz -sy*sz+cz
-sz*cy -sz*sy+cz 

  .. or something like that..

final rotation is Z * Y * X
 cy*cz -sx*-sy*cz+cx*sz  cx*-sy*sz+sx*cz
-cy*sz  sx*sy*sz+cx*cz  -cx*-sy*sz+sx*cz
 sy    -sx*cy            cx*cy

transposed
cy.cz + 0.sz + sy.0            cy.-sz + 0 .cz +  sy.0          cy.0  + 0 .0  +   sy.1       |
sx.sy.cz + cx.sz + -sx.cy.0    sx.sy.-sz + cx.cz + -sx.cy.0    sx.sy.0  + cx.0  + -sx.cy.1  |
-cx.sy.cz + sx.sz +  cx.cy.0   -cx.sy.-sz + sx.cz +  cx.cy.0   -cx.sy.0  + 0 .0  +  cx.cy.1  |
\endverbatim */
inline Matrix4 matrix4_rotation_for_euler_xyz(const Vector3& euler)
{
#if 1

  double cx = cos(euler[0]);
  double sx = sin(euler[0]);
  double cy = cos(euler[1]);
  double sy = sin(euler[1]);
  double cz = cos(euler[2]);
  double sz = sin(euler[2]);

  return Matrix4(
    cy*cz,
    cy*sz,
    -sy,
    0,
    sx*sy*cz + cx*-sz,
    sx*sy*sz + cx*cz,
    sx*cy,
    0,
    cx*sy*cz + sx*sz,
    cx*sy*sz + -sx*cz,
    cx*cy,
    0,
    0,
    0,
    0,
    1
  );

#else

  return matrix4_premultiply_by_matrix4(
    matrix4_premultiply_by_matrix4(
      matrix4_rotation_for_x(euler[0]),
      matrix4_rotation_for_y(euler[1])
    ),
    matrix4_rotation_for_z(euler[2])
  );

#endif
}

/// \brief Constructs a pure-rotation matrix from a set of euler angles (degrees) in the order (x, y, z).
inline Matrix4 matrix4_rotation_for_euler_xyz_degrees(const Vector3& euler)
{
  return matrix4_rotation_for_euler_xyz(euler_degrees_to_radians(euler));
}

/// \brief Concatenates \p self with the rotation transform produced by \p euler angles (degrees) in the order (x, y, z).
/// The concatenated rotation occurs before \p self.
inline void matrix4_rotate_by_euler_xyz_degrees(Matrix4& self, const Vector3& euler)
{
  matrix4_multiply_by_matrix4(self, matrix4_rotation_for_euler_xyz_degrees(euler));
}


/// \brief Constructs a pure-rotation matrix from a set of euler angles (radians) in the order (y, z, x).
inline Matrix4 matrix4_rotation_for_euler_yzx(const Vector3& euler)
{
  return matrix4_premultiplied_by_matrix4(
    matrix4_premultiplied_by_matrix4(
      matrix4_rotation_for_y(euler[1]),
      matrix4_rotation_for_z(euler[2])
    ),
    matrix4_rotation_for_x(euler[0])
  );
}

/// \brief Constructs a pure-rotation matrix from a set of euler angles (degrees) in the order (y, z, x).
inline Matrix4 matrix4_rotation_for_euler_yzx_degrees(const Vector3& euler)
{
  return matrix4_rotation_for_euler_yzx(euler_degrees_to_radians(euler));
}

/// \brief Constructs a pure-rotation matrix from a set of euler angles (radians) in the order (x, z, y).
inline Matrix4 matrix4_rotation_for_euler_xzy(const Vector3& euler)
{
  return matrix4_premultiplied_by_matrix4(
    matrix4_premultiplied_by_matrix4(
      matrix4_rotation_for_x(euler[0]),
      matrix4_rotation_for_z(euler[2])
    ),
    matrix4_rotation_for_y(euler[1])
  );
}

/// \brief Constructs a pure-rotation matrix from a set of euler angles (degrees) in the order (x, z, y).
inline Matrix4 matrix4_rotation_for_euler_xzy_degrees(const Vector3& euler)
{
  return matrix4_rotation_for_euler_xzy(euler_degrees_to_radians(euler));
}

/// \brief Constructs a pure-rotation matrix from a set of euler angles (radians) in the order (y, x, z).
/*! \verbatim
|  cy.cz + sx.sy.-sz + -cx.sy.0   0.cz + cx.-sz + sx.0   sy.cz + -sx.cy.-sz + cx.cy.0 |
|  cy.sz + sx.sy.cz + -cx.sy.0    0.sz + cx.cz + sx.0    sy.sz + -sx.cy.cz + cx.cy.0  |
|  cy.0 + sx.sy.0 + -cx.sy.1      0.0 + cx.0 + sx.1      sy.0 + -sx.cy.0 + cx.cy.1    |
\endverbatim */
inline Matrix4 matrix4_rotation_for_euler_yxz(const Vector3& euler)
{
#if 1

  double cx = cos(euler[0]);
  double sx = sin(euler[0]);
  double cy = cos(euler[1]);
  double sy = sin(euler[1]);
  double cz = cos(euler[2]);
  double sz = sin(euler[2]);

  return Matrix4(
    cy*cz + sx*sy*-sz,
    cy*sz + sx*sy*cz,
    -cx*sy,
    0,
    cx*-sz,
    cx*cz,
    sx,
    0,
    sy*cz + -sx*cy*-sz,
    sy*sz + -sx*cy*cz,
    cx*cy,
    0,
    0,
    0,
    0,
    1
  );

#else

  return matrix4_premultiply_by_matrix4(
    matrix4_premultiply_by_matrix4(
      matrix4_rotation_for_y(euler[1]),
      matrix4_rotation_for_x(euler[0])
    ),
    matrix4_rotation_for_z(euler[2])
  );

#endif
}

/// \brief Constructs a pure-rotation matrix from a set of euler angles (degrees) in the order (y, x, z).
inline Matrix4 matrix4_rotation_for_euler_yxz_degrees(const Vector3& euler)
{
  return matrix4_rotation_for_euler_yxz(euler_degrees_to_radians(euler));
}

/// \brief Returns \p self concatenated with the rotation transform produced by \p euler angles (degrees) in the order (y, x, z).
/// The concatenated rotation occurs before \p self.
inline Matrix4 matrix4_rotated_by_euler_yxz_degrees(const Matrix4& self, const Vector3& euler)
{
  return matrix4_multiplied_by_matrix4(self, matrix4_rotation_for_euler_yxz_degrees(euler));
}

/// \brief Concatenates \p self with the rotation transform produced by \p euler angles (degrees) in the order (y, x, z).
/// The concatenated rotation occurs before \p self.
inline void matrix4_rotate_by_euler_yxz_degrees(Matrix4& self, const Vector3& euler)
{
  self = matrix4_rotated_by_euler_yxz_degrees(self, euler);
}

/// \brief Constructs a pure-rotation matrix from a set of euler angles (radians) in the order (z, x, y).
inline Matrix4 matrix4_rotation_for_euler_zxy(const Vector3& euler)
{
#if 1
  return matrix4_premultiplied_by_matrix4(
    matrix4_premultiplied_by_matrix4(
      matrix4_rotation_for_z(euler[2]),
      matrix4_rotation_for_x(euler[0])
    ),
    matrix4_rotation_for_y(euler[1])
  );
#else
  double cx = cos(euler[0]);
  double sx = sin(euler[0]);
  double cy = cos(euler[1]);
  double sy = sin(euler[1]);
  double cz = cos(euler[2]);
  double sz = sin(euler[2]);

  return Matrix4(
    cz * cy + sz * sx * sy,
    sz * cx,
    cz * -sy + sz * sx * cy,
    0,
    -sz * cy + cz * sx * sy,
    cz * cx,
    -sz * -sy + cz * cx * cy,
    0,
    cx* sy,
    -sx,
    cx* cy,
    0,
    0,
    0,
    0,
    1
  );
#endif
}

/// \brief Constructs a pure-rotation matrix from a set of euler angles (degres=es) in the order (z, x, y).
inline Matrix4 matrix4_rotation_for_euler_zxy_degrees(const Vector3& euler)
{
  return matrix4_rotation_for_euler_zxy(euler_degrees_to_radians(euler));
}

/// \brief Returns \p self concatenated with the rotation transform produced by \p euler angles (degrees) in the order (z, x, y).
/// The concatenated rotation occurs before \p self.
inline Matrix4 matrix4_rotated_by_euler_zxy_degrees(const Matrix4& self, const Vector3& euler)
{
  return matrix4_multiplied_by_matrix4(self, matrix4_rotation_for_euler_zxy_degrees(euler));
}

/// \brief Concatenates \p self with the rotation transform produced by \p euler angles (degrees) in the order (z, x, y).
/// The concatenated rotation occurs before \p self.
inline void matrix4_rotate_by_euler_zxy_degrees(Matrix4& self, const Vector3& euler)
{
  self = matrix4_rotated_by_euler_zxy_degrees(self, euler);
}

/// \brief Constructs a pure-rotation matrix from a set of euler angles (radians) in the order (z, y, x).
inline Matrix4 matrix4_rotation_for_euler_zyx(const Vector3& euler)
{
#if 1

  double cx = cos(euler[0]);
  double sx = sin(euler[0]);
  double cy = cos(euler[1]);
  double sy = sin(euler[1]);
  double cz = cos(euler[2]);
  double sz = sin(euler[2]);

  return Matrix4(
    cy*cz,
    sx*sy*cz + cx*sz,
    cx*-sy*cz + sx*sz,
    0,
    cy*-sz,
    sx*sy*-sz + cx*cz,
    cx*-sy*-sz + sx*cz,
    0,
    sy,
    -sx*cy,
    cx*cy,
    0,
    0,
    0,
    0,
    1
  );

#else

  return matrix4_premultiply_by_matrix4(
    matrix4_premultiply_by_matrix4(
      matrix4_rotation_for_z(euler[2]),
      matrix4_rotation_for_y(euler[1])
    ),
    matrix4_rotation_for_x(euler[0])
  );

#endif
}

/// \brief Constructs a pure-rotation matrix from a set of euler angles (degrees) in the order (z, y, x).
inline Matrix4 matrix4_rotation_for_euler_zyx_degrees(const Vector3& euler)
{
  return matrix4_rotation_for_euler_zyx(euler_degrees_to_radians(euler));
}


/// \brief Calculates and returns a set of euler angles that produce the rotation component of \p self when applied in the order (x, y, z).
/// \p self must be affine and orthonormal (unscaled) to produce a meaningful result.
inline Vector3 matrix4_get_rotation_euler_xyz(const Matrix4& self)
{
  double a = asin(-self[2]);
  double ca = cos(a);

  if (fabs(ca) > 0.005) // Gimbal lock?
  {
    return Vector3(
      atan2(self[6] / ca, self[10] / ca),
      a,
      atan2(self[1] / ca, self[0]/ ca)
    );
  }
  else // Gimbal lock has occurred
  {
    return Vector3(
      atan2(-self[9], self[5]),
      a,
      0
    );
  }
}

/// \brief \copydoc matrix4_get_rotation_euler_xyz(const Matrix4&)
inline Vector3 matrix4_get_rotation_euler_xyz_degrees(const Matrix4& self)
{
  return euler_radians_to_degrees(matrix4_get_rotation_euler_xyz(self));
}

/// \brief Calculates and returns a set of euler angles that produce the rotation component of \p self when applied in the order (y, x, z).
/// \p self must be affine and orthonormal (unscaled) to produce a meaningful result.
inline Vector3 matrix4_get_rotation_euler_yxz(const Matrix4& self)
{
  double a = asin(self[6]);
  double ca = cos(a);

  if (fabs(ca) > 0.005) // Gimbal lock?
  {
    return Vector3(
      a,
      atan2(-self[2] / ca, self[10]/ ca),
      atan2(-self[4] / ca, self[5] / ca)
    );
  }
  else // Gimbal lock has occurred
  {
    return Vector3(
      a,  
      atan2(self[8], self[0]),
      0
    );
  }
}

/// \brief \copydoc matrix4_get_rotation_euler_yxz(const Matrix4&)
inline Vector3 matrix4_get_rotation_euler_yxz_degrees(const Matrix4& self)
{
  return euler_radians_to_degrees(matrix4_get_rotation_euler_yxz(self));
}

/// \brief Calculates and returns a set of euler angles that produce the rotation component of \p self when applied in the order (z, x, y).
/// \p self must be affine and orthonormal (unscaled) to produce a meaningful result.
inline Vector3 matrix4_get_rotation_euler_zxy(const Matrix4& self)
{
  double a = asin(-self[9]);
  double ca = cos(a);

  if (fabs(ca) > 0.005) // Gimbal lock?
  {
    return Vector3(
      a,
      atan2(self[8] / ca, self[10] / ca),
      atan2(self[1] / ca, self[5]/ ca)
    );
  }
  else // Gimbal lock has occurred
  {
    return Vector3(
      a,  
      0,
      atan2(-self[4], self[0])
    );
  }
}

/// \brief \copydoc matrix4_get_rotation_euler_zxy(const Matrix4&)
inline Vector3 matrix4_get_rotation_euler_zxy_degrees(const Matrix4& self)
{
  return euler_radians_to_degrees(matrix4_get_rotation_euler_zxy(self));
}

/// \brief Calculates and returns a set of euler angles that produce the rotation component of \p self when applied in the order (z, y, x).
/// \p self must be affine and orthonormal (unscaled) to produce a meaningful result.
inline Vector3 matrix4_get_rotation_euler_zyx(const Matrix4& self)
{
  double a = asin(self[8]);
  double ca = cos(a);

  if (fabs(ca) > 0.005) // Gimbal lock?
  {
    return Vector3(
      atan2(-self[9] / ca, self[10]/ ca),
      a,
      atan2(-self[4] / ca, self[0] / ca)
    );
  }
  else // Gimbal lock has occurred
  {
    return Vector3(
      0,
      a,
      atan2(self[1], self[5])
    );
  }
}

/// \brief \copydoc matrix4_get_rotation_euler_zyx(const Matrix4&)
inline Vector3 matrix4_get_rotation_euler_zyx_degrees(const Matrix4& self)
{
  return euler_radians_to_degrees(matrix4_get_rotation_euler_zyx(self));
}


/// \brief Rotate \p self by \p euler angles (degrees) applied in the order (x, y, z), using \p pivotpoint.
inline void matrix4_pivoted_rotate_by_euler_xyz_degrees(Matrix4& self, const Vector3& euler, const Vector3& pivotpoint)
{
  matrix4_translate_by_vec3(self, pivotpoint);
  matrix4_rotate_by_euler_xyz_degrees(self, euler);
  matrix4_translate_by_vec3(self, -pivotpoint);
}


/// \brief Constructs a pure-scale matrix from \p scale.
inline Matrix4 matrix4_scale_for_vec3(const Vector3& scale)
{
  return Matrix4(
    scale[0], 0, 0, 0,
    0, scale[1], 0, 0,
    0, 0, scale[2], 0,
    0, 0, 0,        1
  );
}

/// \brief Calculates and returns the (x, y, z) scale values that produce the scale component of \p self.
/// \p self must be affine and orthogonal to produce a meaningful result.
inline Vector3 matrix4_get_scale_vec3(const Matrix4& self)
{
  return Vector3(
    self.x().getVector3().getLength(),
    self.y().getVector3().getLength(),
    self.z().getVector3().getLength()
  );
}

/// \brief Scales \p self by \p scale.
inline void matrix4_scale_by_vec3(Matrix4& self, const Vector3& scale)
{
  matrix4_multiply_by_matrix4(self, matrix4_scale_for_vec3(scale));
}

/// \brief Scales \p self by \p scale, using \p pivotpoint.
inline void matrix4_pivoted_scale_by_vec3(Matrix4& self, const Vector3& scale, const Vector3& pivotpoint)
{
  matrix4_translate_by_vec3(self, pivotpoint);
  matrix4_scale_by_vec3(self, scale);
  matrix4_translate_by_vec3(self, -pivotpoint);
}


/// \brief Transforms \p self by \p translation, \p euler and \p scale.
/// The transforms are combined in the order: scale, rotate-z, rotate-y, rotate-x, translate.
inline void matrix4_transform_by_euler_xyz_degrees(Matrix4& self, const Vector3& translation, const Vector3& euler, const Vector3& scale)
{
  matrix4_translate_by_vec3(self, translation);
  matrix4_rotate_by_euler_xyz_degrees(self, euler);
  matrix4_scale_by_vec3(self, scale);
}

/// \brief Transforms \p self by \p translation, \p euler and \p scale, using \p pivotpoint.
inline void matrix4_pivoted_transform_by_euler_xyz_degrees(Matrix4& self, const Vector3& translation, const Vector3& euler, const Vector3& scale, const Vector3& pivotpoint)
{
  matrix4_translate_by_vec3(self, pivotpoint + translation);
  matrix4_rotate_by_euler_xyz_degrees(self, euler);
  matrix4_scale_by_vec3(self, scale);
  matrix4_translate_by_vec3(self, -pivotpoint);
}

/** Stream insertion operator for Matrix4.
 */
 
inline std::ostream& operator<<(std::ostream& st, const Matrix4& m) {
	st << "|" << m.xx() << ", " << m.xy() << ", " << m.xz() << ", " << m.xw() << "|\n";
	st << "|" << m.yx() << ", " << m.yy() << ", " << m.yz() << ", " << m.yw() << "|\n";
	st << "|" << m.zx() << ", " << m.zy() << ", " << m.zz() << ", " << m.zw() << "|\n";
	st << "|" << m.tx() << ", " << m.ty() << ", " << m.tz() << ", " << m.tw() << "|\n";
	return st;
}

#endif
