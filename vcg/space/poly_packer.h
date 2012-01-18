/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/
#ifndef __VCG_POLY_PACKER_H__
#define __VCG_POLY_PACKER_H__
#include <limits>
#include <stdio.h>
#include <assert.h>
#include <algorithm>
#include <vector>
#include <vcg/space/box2.h>
#include <vcg/space/rect_packer.h>
#include <vcg/space/point2.h>
#include <vcg/math/similarity2.h>

namespace vcg
{

template <class SCALAR_TYPE>
class PolyPacker
{
  typedef typename vcg::Box2<SCALAR_TYPE> Box2x;
  typedef typename vcg::Point2<SCALAR_TYPE> Point2x;
  typedef typename vcg::Similarity2<SCALAR_TYPE> Similarity2x;

public:

  static Box2f getPolyBB(const std::vector<Point2x> &poly)
  {
    Box2f bb;
    for(size_t i=0;i<poly.size();++i)
      bb.Add(poly[i]);

    return bb;
  }

  static Box2f getPolyOOBB(const std::vector<Point2x> &poly, float &rot)
  {
    const int stepNum=16;
    float bestAngle;
    float bestArea = std::numeric_limits<float>::max();
    Box2f bestBB;

    for(int i=0;i<stepNum;++i)
    {
      float angle = float(i)*(M_PI/2.0)/float(stepNum);
      Box2f bb;
      for(size_t i=0;i<poly.size();++i)
      {
        Point2f pp=poly[i];
        pp.Rotate(angle);
        bb.Add(pp);
      }

      if(bb.Area()<bestArea)
      {
        bestAngle=angle;
        bestArea=bb.Area();
        bestBB=bb;
      }
    }
    rot=bestAngle;
    return bestBB;
  }

static  bool PackAsEqualSquares(const std::vector< std::vector<Point2x> > &polyVec,
                  const Point2x containerSizeX,
                  std::vector<Similarity2x> &trVec,
                  Point2x &coveredContainer)
{
  int minSide = int( std::min(containerSizeX[0],containerSizeX[1]));
  const vcg::Point2i containerSize(minSide,minSide);
  int polyPerLine = ceil(sqrt(polyVec.size()));
  int pixelPerPoly = minSide / (polyPerLine);
  if(pixelPerPoly < 1) return false;

  trVec.clear();
  trVec.resize(polyVec.size());
  Box2f bbMax;
  std::vector<Box2x> bbVec;
  for(size_t i=0;i<polyVec.size();++i)
  {
    bbVec.push_back(getPolyBB(polyVec[i]));
    bbMax.Add(bbVec.back());
  }

  float unitScale = 1.0/std::max(bbMax.DimX(),bbMax.DimY());
  float polyScale = unitScale * pixelPerPoly;

  int baseX =0;
  int baseY=0;
  for(size_t i=0;i<polyVec.size();++i)
  {
    trVec[i].sca = polyScale; // the same scaling for all the polygons;
    trVec[i].tra = Point2f(baseX+(0.5*pixelPerPoly), baseY+(0.5*pixelPerPoly)) - bbVec[i].Center()*polyScale;
    baseX +=pixelPerPoly;

    if(baseX +pixelPerPoly>minSide)
    {
      baseY+=pixelPerPoly;
      baseX=0;
    }
  }
  return true;
}

static bool PackAsAxisAlignedRect(const std::vector< std::vector<Point2x> > &polyVec,
                  const Point2x containerSizeX,
                  std::vector<Similarity2x> &trVec,
                  Point2x &coveredContainer)
{
  trVec.clear();
  trVec.resize(polyVec.size());
  std::vector<Box2x> bbVec;
  for(size_t i=0;i<polyVec.size();++i)
  {
    assert(polyVec[i].size()>0);
    bbVec.push_back(getPolyBB(polyVec[i]));
  }
  return RectPacker<float>::Pack(bbVec,containerSizeX,trVec,coveredContainer);
}

static bool PackAsObjectOrientedRect(const std::vector< std::vector<Point2x> > &polyVec,
                  const Point2x containerSizeX,
                  std::vector<Similarity2x> &trVec,
                  Point2x &coveredContainer)
{
  trVec.clear();
  trVec.resize(polyVec.size());
  std::vector<Box2x> bbVec;
  std::vector<float> rotVec;
  for(size_t i=0;i<polyVec.size();++i)
  {
    float rot;
    bbVec.push_back(getPolyOOBB(polyVec[i],rot));
    rotVec.push_back(rot);
  }

  bool ret= RectPacker<float>::Pack(bbVec,containerSizeX,trVec,coveredContainer);

  for(size_t i=0;i<polyVec.size();++i)
  {
    trVec[i].rotRad=rotVec[i];
  }
  return ret;
}

}; // end class
} // end namespace vcg
#endif // POLY_PACKER_H
