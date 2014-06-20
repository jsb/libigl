#include "cross_field_missmatch.h"
#include "comb_cross_field.h"

#include <vector>
#include <deque>
#include "per_face_normals.h"
#include "is_border_vertex.h"
#include "vf.h"
#include "tt.h"


namespace igl {
  template <typename DerivedV, typename DerivedF, typename DerivedO>
  class MissMatchCalculator
  {
  public:
    
    const Eigen::PlainObjectBase<DerivedV> &V;
    const Eigen::PlainObjectBase<DerivedF> &F;
    const Eigen::PlainObjectBase<DerivedV> &PD1;
    const Eigen::PlainObjectBase<DerivedV> &PD2;
    Eigen::PlainObjectBase<DerivedV> N;
    
  private:
    // internal
    std::vector<bool> V_border; // bool
    std::vector<std::vector<int> > VF;
    std::vector<std::vector<int> > VFi;
    Eigen::PlainObjectBase<DerivedF> TT;
    Eigen::PlainObjectBase<DerivedF> TTi;
    
    
  private:
    
    ///return true if a vertex is singluar by looking at initialized missmatches
    // possible bugs, verify deleted flag vs IsD()
    // not sorted vf, but should not make a difference
    // olga: TODO: this returns the index modulo 4.
    int oneRingMissMatch(const int vid)
    {
      ///check that is on border..
      if (V_border[vid])
        return 0;
      
      int missmatch=0;
      for (unsigned int i=0;i<VF[vid].size();i++)
      {
        // look for the vertex
        int j=-1;
        for (unsigned z=0; z<3; ++z)
          if (F(VF[vid][i],z) == vid)
            j=z;
        assert(j!=-1);
        
        missmatch+=Handle_MMatch(VF[vid][i],j);
      }
      
      missmatch=missmatch%4;
      return missmatch;
    }
    
    
    ///compute the mismatch between 2 faces
    int MissMatchByCross(const int f0,
                         const int f1)
    {
      Eigen::Matrix<typename DerivedV::Scalar, 3, 1> dir0 = PD1.row(f0);
      Eigen::Matrix<typename DerivedV::Scalar, 3, 1> dir1 = PD1.row(f1);
      Eigen::Matrix<typename DerivedV::Scalar, 3, 1> n0 = N.row(f0);
      Eigen::Matrix<typename DerivedV::Scalar, 3, 1> n1 = N.row(f1);
      
      Eigen::Matrix<typename DerivedV::Scalar, 3, 1> dir1Rot = Comb<DerivedV, DerivedF>::Rotate(n1,n0,dir1);
      dir1Rot.normalize();
      
      // TODO: this should be equivalent to the other code below, to check!
      // Compute the angle between the two vectors
      //    double a0 = atan2(dir0.dot(B2.row(f0)),dir0.dot(B1.row(f0)));
      //    double a1 = atan2(dir1Rot.dot(B2.row(f0)),dir1Rot.dot(B1.row(f0)));
      //
      //    double angle_diff = a1-a0;   //VectToAngle(f0,dir1Rot);
      
      double angle_diff = atan2(dir1Rot.dot(PD2.row(f0)),dir1Rot.dot(PD1.row(f0)));
      
      //    std::cerr << "Dani: " << dir0(0) << " " << dir0(1) << " " << dir0(2) << " " << dir1Rot(0) << " " << dir1Rot(1) << " " << dir1Rot(2) << " " << angle_diff << std::endl;
      
      double step=M_PI/2.0;
      int i=(int)floor((angle_diff/step)+0.5);
      int k=0;
      if (i>=0)
        k=i%4;
      else
        k=(-(3*i))%4;
      return k;
    }
    
    
public:
  MissMatchCalculator(const Eigen::PlainObjectBase<DerivedV> &_V,
                      const Eigen::PlainObjectBase<DerivedF> &_F,
                      const Eigen::PlainObjectBase<DerivedV> &_PD1,
                      const Eigen::PlainObjectBase<DerivedV> &_PD2
                      ):
  V(_V),
  F(_F),
  PD1(_PD1),
  PD2(_PD2)
  {
    igl::per_face_normals(V,F,N);
    V_border = igl::is_border_vertex(V,F);
    igl::vf(V,F,VF,VFi);
    igl::tt(V,F,TT,TTi);
  }
  
  void calculateMissmatch(Eigen::PlainObjectBase<DerivedO> &Handle_MMatch)
  {
    Handle_MMatch.setConstant(F.rows(),3,-1);
    for (unsigned int i=0;i<F.rows();i++)
    {
      for (int j=0;j<3;j++)
      {
        if (i==TT(i,j) || TT(i,j) == -1)
          Handle_MMatch(i,j)=0;
        else
          Handle_MMatch(i,j) = MissMatchByCross(i,TT(i,j));
      }
    }
  }
  
};
}
template <typename DerivedV, typename DerivedF, typename DerivedO>
IGL_INLINE void igl::cross_field_missmatch(const Eigen::PlainObjectBase<DerivedV> &V,
                                           const Eigen::PlainObjectBase<DerivedF> &F,
                                           const Eigen::PlainObjectBase<DerivedV> &PD1,
                                           const Eigen::PlainObjectBase<DerivedV> &PD2,
                                           const bool isCombed,
                                           Eigen::PlainObjectBase<DerivedO> &missmatch)
{
  Eigen::PlainObjectBase<DerivedV> PD1_combed;
  Eigen::PlainObjectBase<DerivedV> PD2_combed;
  
  if (!isCombed)
    igl::comb_cross_field(V,F,PD1,PD2,PD1_combed,PD2_combed);
  else
  {
    PD1_combed = PD1;
    PD2_combed = PD2;
  }
  igl::MissMatchCalculator<DerivedV, DerivedF, DerivedO> sf(V, F, PD1_combed, PD2_combed);
  sf.calculateMissmatch(missmatch);
}