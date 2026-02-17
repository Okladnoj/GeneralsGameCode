/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S
 ****
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : wwmath *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWMath/sphere.h $*
 *                                                                                             *
 *                       Author:: Greg_h *
 *                                                                                             *
 *                     $Modtime:: 3/29/01 10:37a $*
 *                                                                                             *
 *                    $Revision:: 18 $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions: * SphereClass::SphereClass -- constructor * SphereClass::Init --
 *assign a new center and radius to this sphere                        *
 *   SphereClass::Re_Center -- move the center, update radius to enclose old
 *sphere            * SphereClass::Add_Sphere -- expands 'this' sphere to
 *enclose the given sphere              * SphereClass::Transform -- transforms
 *this sphere                                          * SphereClass::Volume --
 *returns the volume of this sphere                                  *
 *   SphereClass::operator+= -- 'Add' a sphere to this one *
 *   SphereClass::operator *= -- transform this sphere by the given radius *
 *   Spheres_Intersect -- test whether two spheres intersect * Add_Spheres --
 *Add two spheres together, creating sphere which encloses both              *
 *   operator + -- Add two spheres together, creating a sphere which encloses
 *both             * Transform Sphere -- transform a sphere * Transform_Sphere
 *-- transform a sphere                                                    *
 *   operator * -- Transform a sphere *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *- - - - - - - */

#pragma once

#include "always.h"
#include "matrix3d.h"
#include "vector3.h"

/////////////////////////////////////////////////////////////////////
// SphereClass
//
// Center - center of the sphere.
// Radius - radius of the sphere
//
/////////////////////////////////////////////////////////////////////
class SphereClass {
public:
  SphereClass(void) {};
  SphereClass(const Vector3 &center, float radius) { Init(center, radius); }
  SphereClass(const Matrix3D &mtx, const Vector3 &center, float radius) {
    Init(mtx, center, radius);
  }

  SphereClass(const Vector3 &center, const SphereClass &s0) {
    float dist = (s0.Center - center).Length();
    Center = center;
    Radius = s0.Radius + dist;
  }

  SphereClass(const Vector3 *Position, const int VertCount) {
    int i;
    double dx, dy, dz;

    Vector3 xmin(Position[0].X, Position[0].Y, Position[0].Z);
    Vector3 xmax(Position[0].X, Position[0].Y, Position[0].Z);
    Vector3 ymin(Position[0].X, Position[0].Y, Position[0].Z);
    Vector3 ymax(Position[0].X, Position[0].Y, Position[0].Z);
    Vector3 zmin(Position[0].X, Position[0].Y, Position[0].Z);
    Vector3 zmax(Position[0].X, Position[0].Y, Position[0].Z);

    for (i = 1; i < VertCount; i++) {
      if (Position[i].X < xmin.X) {
        xmin.X = Position[i].X;
        xmin.Y = Position[i].Y;
        xmin.Z = Position[i].Z;
      }
      if (Position[i].X > xmax.X) {
        xmax.X = Position[i].X;
        xmax.Y = Position[i].Y;
        xmax.Z = Position[i].Z;
      }
      if (Position[i].Y < ymin.Y) {
        ymin.X = Position[i].X;
        ymin.Y = Position[i].Y;
        ymin.Z = Position[i].Z;
      }
      if (Position[i].Y > ymax.Y) {
        ymax.X = Position[i].X;
        ymax.Y = Position[i].Y;
        ymax.Z = Position[i].Z;
      }
      if (Position[i].Z < zmin.Z) {
        zmin.X = Position[i].X;
        zmin.Y = Position[i].Y;
        zmin.Z = Position[i].Z;
      }
      if (Position[i].Z > zmax.Z) {
        zmax.X = Position[i].X;
        zmax.Y = Position[i].Y;
        zmax.Z = Position[i].Z;
      }
    }

    dx = xmax.X - xmin.X;
    dy = xmax.Y - xmin.Y;
    dz = xmax.Z - xmin.Z;
    double xspan = dx * dx + dy * dy + dz * dz;

    dx = ymax.X - ymin.X;
    dy = ymax.Y - ymin.Y;
    dz = ymax.Z - ymin.Z;
    double yspan = dx * dx + dy * dy + dz * dz;

    dx = zmax.X - zmin.X;
    dy = zmax.Y - zmin.Y;
    dz = zmax.Z - zmin.Z;
    double zspan = dx * dx + dy * dy + dz * dz;

    Vector3 dia1 = xmin;
    Vector3 dia2 = xmax;
    double maxspan = xspan;

    if (yspan > maxspan) {
      maxspan = yspan;
      dia1 = ymin;
      dia2 = ymax;
    }
    if (zspan > maxspan) {
      maxspan = zspan;
      dia1 = zmin;
      dia2 = zmax;
    }

    Vector3 center;
    center.X = (dia1.X + dia2.X) / 2.0f;
    center.Y = (dia1.Y + dia2.Y) / 2.0f;
    center.Z = (dia1.Z + dia2.Z) / 2.0f;

    dx = dia2.X - center.X;
    dy = dia2.Y - center.Y;
    dz = dia2.Z - center.Z;

    double radsqr = dx * dx + dy * dy + dz * dz;
    double radius = sqrt(radsqr);

    for (i = 0; i < VertCount; i++) {
      dx = Position[i].X - center.X;
      dy = Position[i].Y - center.Y;
      dz = Position[i].Z - center.Z;

      double testrad2 = dx * dx + dy * dy + dz * dz;

      if (testrad2 > radsqr) {
        double testrad = sqrt(testrad2);
        radius = (radius + testrad) / 2.0;
        radsqr = radius * radius;

        double oldtonew = testrad - radius;
        center.X = (radius * center.X + oldtonew * Position[i].X) / testrad;
        center.Y = (radius * center.Y + oldtonew * Position[i].Y) / testrad;
        center.Z = (radius * center.Z + oldtonew * Position[i].Z) / testrad;
      }
    }

    Center = center;
    Radius = radius;
  }

  void Init(const Vector3 &pos, float radius) {
    Center = pos;
    Radius = radius;
  }

  void Init(const Matrix3D &mtx, const Vector3 &pos, float radius) {
#ifdef ALLOW_TEMPORARIES
    Center = mtx * pos;
#else
    mtx.mulVector3(pos, Center);
#endif
    Radius = radius;
  }

  void Re_Center(const Vector3 &center) {
    float dist = (Center - center).Length();
    Center = center;
    Radius += dist;
  }

  void Add_Sphere(const SphereClass &s) {
    if (s.Radius == 0.0f)
      return;

    float dist = (s.Center - Center).Length();
    if (dist == 0.0f) {
      Radius = (Radius > s.Radius) ? Radius : s.Radius;
      return;
    }

    float rnew = (dist + Radius + s.Radius) / 2.0f;

    if (rnew < Radius) {
    } else {
      if (rnew < s.Radius) {
        Init(s.Center, s.Radius);
      } else {
        float lerp = (rnew - Radius) / dist;
        Vector3 center = (s.Center - Center) * lerp + Center;
        Init(center, rnew);
      }
    }
  }

  void Transform(const Matrix3D &tm) {
#ifdef ALLOW_TEMPORARIES
    Center = tm * Center;
#else
    tm.mulVector3(Center, Center);
#endif
  }

  float Volume(void) const {
    return (4.0 / 3.0) * WWMATH_PI * (Radius * Radius * Radius);
  }

  SphereClass &operator+=(const SphereClass &s) {
    Add_Sphere(s);
    return *this;
  }

  SphereClass &operator*=(const Matrix3D &m) {
    Init(m, Center, Radius);
    return *this;
  }

  Vector3 Center;
  float Radius;
};

inline bool Spheres_Intersect(const SphereClass &s0, const SphereClass &s1) {
  Vector3 delta = s0.Center - s1.Center;
  float dist2 = Vector3::Dot_Product(delta, delta);

  if (dist2 < (s0.Radius + s1.Radius) * (s0.Radius + s1.Radius)) {
    return true;
  } else {
    return false;
  }
}

inline SphereClass Add_Spheres(const SphereClass &s0, const SphereClass &s1) {
  if (s0.Radius == 0.0f) {
    return s1;
  } else {
    SphereClass result(s0);
    result.Add_Sphere(s1);
    return result;
  }
}

inline SphereClass operator+(const SphereClass &s0, const SphereClass &s1) {
  return Add_Spheres(s0, s1);
}

inline SphereClass Transform_Sphere(const Matrix3D &m, const SphereClass &s) {
  return SphereClass(m, s.Center, s.Radius);
}

inline void Transform_Sphere(const Matrix3D &m, const SphereClass &s,
                             SphereClass &res) {
#ifdef ALLOW_TEMPORARIES
  res.Center = m * s.Center;
#else
  m.mulVector3(s.Center, res.Center);
#endif
  res.Radius = s.Radius;
}

inline SphereClass operator*(const Matrix3D &m, const SphereClass &s) {
  return Transform_Sphere(m, s);
}
