// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2014 Daniele Panozzo <daniele.panozzo@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#include "areamatrix.h"
#include <vector>

// Bug in unsupported/Eigen/SparseExtra needs iostream first
#include <iostream>
#include <unsupported/Eigen/SparseExtra>

#include <igl/boundary_vertices_sorted.h>

template <typename DerivedV, typename DerivedF, typename Scalar>
IGL_INLINE void igl::areamatrix(
  const Eigen::PlainObjectBase<DerivedV> & V,
  const Eigen::PlainObjectBase<DerivedF> & F,
  Eigen::SparseMatrix<Scalar>& A)
{
  using namespace Eigen;
  using namespace std;

	SparseMatrix<Scalar> aux (V.rows() * 2, V.rows() * 2);
	SparseMatrix<Scalar> auxT(V.rows() * 2, V.rows() * 2);

	vector<Triplet<Scalar> > auxTripletList;
	vector<Triplet<Scalar> > auxTTripletList;

  Eigen::VectorXi bnd;
  igl::boundary_vertices_sorted(V,F,bnd);

	for(int k = 0; k < bnd.size(); k++)
  {
		int i = bnd[k];
		int j = k + 1 == bnd.size() ? bnd[0] : bnd[k+1];
		auxTripletList.push_back(Triplet<Scalar>(i+V.rows(), j, -0.5));
		auxTripletList.push_back(Triplet<Scalar>(i, j+V.rows(), 0.5));
		auxTTripletList.push_back(Triplet<Scalar>(j, i+V.rows(), -0.5));
		auxTTripletList.push_back(Triplet<Scalar>(j+V.rows(), i, 0.5));
	}

	aux.setFromTriplets(auxTripletList.begin(), auxTripletList.end());
	auxT.setFromTriplets(auxTTripletList.begin(), auxTTripletList.end());
	A = (aux + auxT)/2;
}

#ifndef IGL_HEADER_ONLY
// Explicit template specialization
// generated by autoexplicit.sh
#endif