/* Authors: Lutong Wang and Bangqi Xu */
/*
 * Copyright (c) 2019, The Regents of the University of California
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include "gc/FlexGC.h"

using namespace std;
using namespace fr;

void FlexGCWorker::myBloat(const gtl::rectangle_data<frCoord> &rect, frCoord val, box_t &box) {
  bg::set<bg::min_corner, 0>(box, gtl::xl(rect) - val);
  bg::set<bg::min_corner, 1>(box, gtl::yl(rect) - val);
  bg::set<bg::max_corner, 0>(box, gtl::xh(rect) + val);
  bg::set<bg::max_corner, 1>(box, gtl::yh(rect) + val);
}

frCoord FlexGCWorker::checkMetalSpacing_getMaxSpcVal(frLayerNum layerNum) {
  frCoord maxSpcVal = 0;
  auto currLayer = getDesign()->getTech()->getLayer(layerNum);
  if (currLayer->hasMinSpacing()) {
    auto con = currLayer->getMinSpacing();
    switch (con->typeId()) {
      case frConstraintTypeEnum::frcSpacingConstraint:
        maxSpcVal = static_cast<frSpacingConstraint*>(con)->getMinSpacing();
        break;
      case frConstraintTypeEnum::frcSpacingTablePrlConstraint:
        maxSpcVal = static_cast<frSpacingTablePrlConstraint*>(con)->findMax();
        break;
      default:
        std::cout << "Warning: Unsupported metSpc rule\n";
    }
  }
  return maxSpcVal;
}

frCoord FlexGCWorker::checkMetalSpacing_prl_getReqSpcVal(gcRect* rect1, gcRect* rect2, frCoord prl/*, bool &hasRoute*/) {
  auto layerNum = rect1->getLayerNum();
  frCoord reqSpcVal = 0;
  auto currLayer = getDesign()->getTech()->getLayer(layerNum);
  bool isObs  = false;
  //bool hasOverrideSpacing = false;
  //bool hasOverrideWidth = false;
  //frCoord overrideSpacingVal = 0;
  auto width1 = rect1->width();
  auto width2 = rect2->width();
  // override width and spacing
  if (rect1->getNet()->getOwner() && 
      (rect1->getNet()->getOwner()->typeId() == frcInstBlockage || rect1->getNet()->getOwner()->typeId() == frcBlockage)) {
    isObs = true;
    if (USEMINSPACING_OBS) {
      width1 = currLayer->getWidth();
    }
    //frBlockage* obs1 = nullptr;
    //if (rect1->getNet()->getOwner()->typeId() == frcInstBlockage) {
    //  obs1 = static_cast<frInstBlockage*>(rect1->getNet()->getOwner())->getBlockage();
    //} else if (rect1->getNet()->getOwner()->typeId() == frcBlockage) {
    //  obs1 = static_cast<frBlockage*>(rect1->getNet()->getOwner());
    //}
    //if (obs1->hasSpacing(rect1->getLayerNum())) {
    //  hasOverrideSpacing = true;
    //  overrideSpacingVal = std::max(overrideSpacingVal, obs1->getSpacing(rect1->getLayerNum()));
    //}
    //if (obs1->hasDesignRuleWidth(rect1->getLayerNum())) {
    //  hasOverrideWidth = true;
    //  width1 = obs1->getDesignRuleWidth(rect1->getLayerNum());
    //}
  }
  if (rect2->getNet()->getOwner() && 
      (rect2->getNet()->getOwner()->typeId() == frcInstBlockage || rect2->getNet()->getOwner()->typeId() == frcBlockage)) {
    isObs = true;
    if (USEMINSPACING_OBS) {
      width2 = currLayer->getWidth();
    }
    //frBlockage* obs1 = nullptr;
    //if (rect2->getNet()->getOwner()->typeId() == frcInstBlockage) {
    //  obs1 = static_cast<frInstBlockage*>(rect2->getNet()->getOwner())->getBlockage();
    //} else if (rect1->getNet()->getOwner()->typeId() == frcBlockage) {
    //  obs1 = static_cast<frBlockage*>(rect2->getNet()->getOwner());
    //}
    //if (obs1->hasSpacing(rect2->getLayerNum())) {
    //  hasOverrideSpacing = true;
    //  overrideSpacingVal = std::max(overrideSpacingVal, obs1->getSpacing(rect2->getLayerNum()));
    //}
    //if (obs1->hasDesignRuleWidth(rect2->getLayerNum())) {
    //  hasOverrideWidth = true;
    //  width2 = obs1->getDesignRuleWidth(rect2->getLayerNum());
    //}
  }
  // check if width is a result of route shape
  // if the width a shape is smaller if only using fixed shape, then it's route shape -- wrong...
  if (currLayer->hasMinSpacing()) {
    auto con = currLayer->getMinSpacing();
    switch (con->typeId()) {
      //case frConstraintTypeEnum::frcSpacingConstraint:
      //  reqSpcVal = static_cast<frSpacingConstraint*>(con)->getMinSpacing();
      //  break;
      case frConstraintTypeEnum::frcSpacingTablePrlConstraint:
        reqSpcVal = static_cast<frSpacingTablePrlConstraint*>(con)->find(std::max(width1, width2), prl);
        // obs override
        //if (isObs) {
        //  if (hasOverrideSpacing) {
        //    reqSpcVal = overrideSpacingVal;
        //  }
        //}
        // same-net override
        if (!isObs && rect1->getNet() == rect2->getNet()) {
          if (currLayer->hasSpacingSamenet()) {
            auto conSamenet = currLayer->getSpacingSamenet();
            if (!conSamenet->hasPGonly()) {
              reqSpcVal = std::max(conSamenet->getMinSpacing(), static_cast<frSpacingTablePrlConstraint*>(con)->findMin());
            } else {
              bool isPG = false;
              auto owner = rect1->getNet()->getOwner();
              if (owner->typeId() == frcNet) {
                if (static_cast<frNet*>(owner)->getType() == frNetEnum::frcPowerNet || 
                    static_cast<frNet*>(owner)->getType() == frNetEnum::frcGroundNet) {
                  isPG = true;
                }
              } else if (owner->typeId() == frcInstTerm) {
                if (static_cast<frInstTerm*>(owner)->getTerm()->getType() == frTermEnum::frcPowerTerm || 
                    static_cast<frInstTerm*>(owner)->getTerm()->getType() == frTermEnum::frcGroundTerm) {
                  isPG = true;
                }
              } else if (owner->typeId() == frcTerm) {
                if (static_cast<frTerm*>(owner)->getType() == frTermEnum::frcPowerTerm || 
                    static_cast<frTerm*>(owner)->getType() == frTermEnum::frcGroundTerm) {
                  isPG = true;
                }
              }
              if (isPG) {
                reqSpcVal = std::max(conSamenet->getMinSpacing(), static_cast<frSpacingTablePrlConstraint*>(con)->findMin());
              }
            }
          }
        }
        break;
      default:
        std::cout << "Warning: Unsupported metSpc rule\n";
    }
  }
  return reqSpcVal;
}

// type: 0 -- check H edge only
// type: 1 -- check V edge only
// type: 2 -- check both
bool FlexGCWorker::checkMetalSpacing_prl_hasPolyEdge(gcRect* rect1, gcRect* rect2, const gtl::rectangle_data<frCoord> &markerRect, int type, frCoord prl) {
  auto layerNum = rect1->getLayerNum();
  auto net1 = rect1->getNet();
  auto net2 = rect2->getNet();
  auto &workerRegionQuery = getWorkerRegionQuery();
  vector<pair<segment_t, gcSegment*> > result;
  box_t queryBox(point_t(gtl::xl(markerRect), gtl::yl(markerRect)), 
                 point_t(gtl::xh(markerRect), gtl::yh(markerRect)));
  workerRegionQuery.queryPolygonEdge(queryBox, layerNum, result);
  // whether markerRect edge has true poly edge of either net1 or net2
  bool flagL = false;
  bool flagR = false;
  bool flagB = false;
  bool flagT = false;
  // type == 2 allows zero overlapping
  if (prl <= 0) {
    for (auto &[seg, objPtr]: result) {
      if ((objPtr->getNet() != net1 && objPtr->getNet() != net2)) {
        continue;
      }
      if (objPtr->getDir() == frDirEnum::W && objPtr->low().y() == gtl::yl(markerRect)) {
        flagB = true;
      } else if (objPtr->getDir() == frDirEnum::E && objPtr->low().y() == gtl::yh(markerRect)) {
        flagT = true;
      } else if (objPtr->getDir() == frDirEnum::N && objPtr->low().x() == gtl::xl(markerRect)) { // mod 8/22/19
        flagL = true;
      } else if (objPtr->getDir() == frDirEnum::S && objPtr->low().x() == gtl::xh(markerRect)) { // mod 8/22/19
        flagR = true;
      }
    }
  // type == 0 / 1 requires non-zero overlapping poly edge 
  } else {
    for (auto &[seg, objPtr]: result) {
      if ((objPtr->getNet() != net1 && objPtr->getNet() != net2)) {
        continue;
      }
      // allow fake edge if inside markerRect has same dir edge
      if (objPtr->getDir() == frDirEnum::W && 
          (objPtr->low().y() >= gtl::yl(markerRect) && objPtr->low().y() < gtl::yh(markerRect)) &&
          !(objPtr->low().x() <= gtl::xl(markerRect) || objPtr->high().x() >= gtl::xh(markerRect))) {
        flagB = true;
      } else if (objPtr->getDir() == frDirEnum::E && 
                 (objPtr->low().y() > gtl::yl(markerRect) && objPtr->low().y() <= gtl::yh(markerRect)) &&
                 !(objPtr->high().x() <= gtl::xl(markerRect) || objPtr->low().x() >= gtl::xh(markerRect))) {
        flagT = true;
      } else if (objPtr->getDir() == frDirEnum::N && 
                 (objPtr->low().x() >= gtl::xl(markerRect) && objPtr->low().x() < gtl::xh(markerRect)) &&
                 !(objPtr->high().y() <= gtl::yl(markerRect) || objPtr->low().y() >= gtl::yh(markerRect))) {
        flagL = true;
      } else if (objPtr->getDir() == frDirEnum::S && 
                 (objPtr->low().x() > gtl::xl(markerRect) && objPtr->low().x() <= gtl::xh(markerRect)) && 
                 !(objPtr->low().y() <= gtl::yl(markerRect) || objPtr->high().y() >= gtl::yh(markerRect))) {
        flagR = true;
      }
    }
  }
  if ((type == 0 || type == 2) && (flagB && flagT)) {
    return true;
  } else if ((type == 1 || type == 2) && (flagL && flagR)) {
    return true;
  } else {
    return false;
  }
}

void FlexGCWorker::checkMetalSpacing_prl(gcRect* rect1, gcRect* rect2, const gtl::rectangle_data<frCoord> &markerRect,
                                     frCoord prl, frCoord distX, frCoord distY) {
  bool enableOutput = printMarker;

  auto layerNum = rect1->getLayerNum();
  auto net1 = rect1->getNet();
  auto net2 = rect2->getNet();
  //bool hasRoute = false;
  auto reqSpcVal = checkMetalSpacing_prl_getReqSpcVal(rect1, rect2, prl);
  bool testflag = false;
  //if (gtl::xl(markerRect) == frCoord(264.01  * 2000) &&
  //    gtl::yl(markerRect) == frCoord(62.72   * 2000) &&
  //    gtl::xh(markerRect) == frCoord(264.445 * 2000) &&
  //    gtl::yh(markerRect) == frCoord(62.87   * 2000)) {
  //  cout <<"@@@0" <<endl;
  //  testflag = true;
  //}
  //if (layerNum == 0 &&
  //    gtl::xl(markerRect) == frCoord(210.370 * 2000) &&
  //    gtl::yl(markerRect) == frCoord( 96.455 * 2000) &&
  //    gtl::xh(markerRect) == frCoord(210.405 * 2000) &&
  //    gtl::yh(markerRect) == frCoord( 96.455 * 2000)) {
  //  cout <<"@@@0" <<endl;
  //  testflag = true;
  //}
  // no violation if fixed shapes
  if (rect1->isFixed() && rect2->isFixed()) {
    if (testflag) {
      cout <<"@@@1" <<endl;
    }
    return;
  }
  // no violation if spacing satisfied
  if (distX * distX + distY * distY >= reqSpcVal * reqSpcVal) {
    if (testflag) {
      cout <<"@@@2 " <<reqSpcVal <<endl;
    }
    return;
  }
  // no violation if no two true polygon edges (prl > 0 requires non-zero true poly edge; prl <= 0 allows zero true poly edge)
  int type = 0;
  if (prl <= 0) {
    type = 2;
  } else {
    if (distX == 0) {
      type = 0;
    } else if (distY == 0) {
      type = 1;
    }
  }
  if (!checkMetalSpacing_prl_hasPolyEdge(rect1, rect2, markerRect, type, prl)) {
    if (testflag) {
      cout <<"@@@3" <<endl;
    }
    return;
  }
  // no violation if covered by shapes of the two nets 
  // <--> (prl > 0) edge && markerRect should result in non-zero true edge length
  //auto &workerRegionQuery = getWorkerRegionQuery();
  //vector<rq_rptr_value_t<gcRect> > result;
  //box_t queryBox(point_t(gtl::xl(markerRect), gtl::yl(markerRect)), 
  //               point_t(gtl::xh(markerRect), gtl::yh(markerRect)));
  //workerRegionQuery.queryMaxRectangle(queryBox, layerNum, result);
  //for (auto &[objBox, objPtr]: result) {
  //  if ((objPtr->getNet() == net1 || objPtr->getNet() == net2) && bg::covered_by(queryBox, objBox)) {
  //    if (testflag) {
  //      cout <<"@@@4" <<endl;
  //    }
  //    return;
  //  }
  //}
  // no violation if bloat width cannot find route shapes
  //{
  //  using namespace boost::polygon::operators;
  //  auto width = std::max(rect1->width(), rect2->width());
  //  auto width = std::max(rect1->width(), rect2->width());
  //  gtl::rectangle_data<frCoord> enlargedMarkerRect(markerRect);
  //  gtl::bloat(enlargedMarkerRect, width);
  //  auto &polys1 = net1->getPolygons(layerNum, false);
  //  auto intersection_polys1 = polys1 & enlargedMarkerRect;
  //  auto &polys2 = net2->getPolygons(layerNum, false);
  //  auto intersection_polys2 = polys2 & enlargedMarkerRect;
  //  if (gtl::empty(intersection_polys1) && gtl::empty(intersection_polys2)) {
  //    if (testflag) {
  //      cout <<"@@@5" <<endl;
  //    }
  //    return;
  //  }
  //}
  // no violation if bloat width cannot find non-fixed route shapes
  bool hasRoute = false;
  if (!hasRoute) {
    // marker enlarged by width
    auto width = rect1->width();
    gtl::rectangle_data<frCoord> enlargedMarkerRect(markerRect);
    gtl::bloat(enlargedMarkerRect, width);
    // widthrect
    gtl::polygon_90_set_data<frCoord> tmpPoly;
    using namespace boost::polygon::operators;
    tmpPoly += enlargedMarkerRect;
    tmpPoly &= *rect1; // tmpPoly now is widthrect
    auto targetArea = gtl::area(tmpPoly);
    // get fixed shapes
    tmpPoly &= net1->getPolygons(layerNum, true);
    if (gtl::area(tmpPoly) < targetArea) {
      hasRoute = true;
    }
  }
  if (!hasRoute) {
    // marker enlarged by width
    auto width = rect2->width();
    gtl::rectangle_data<frCoord> enlargedMarkerRect(markerRect);
    gtl::bloat(enlargedMarkerRect, width);
    // widthrect
    gtl::polygon_90_set_data<frCoord> tmpPoly;
    using namespace boost::polygon::operators;
    tmpPoly += enlargedMarkerRect;
    tmpPoly &= *rect2; // tmpPoly now is widthrect
    auto targetArea = gtl::area(tmpPoly);
    // get fixed shapes
    tmpPoly &= net2->getPolygons(layerNum, true);
    if (gtl::area(tmpPoly) < targetArea) {
      hasRoute = true;
    }
  }
  if (!hasRoute) {
    if (testflag) {
      cout <<"@@@5" <<endl;
    }
    return;
  }

  auto marker = make_unique<frMarker>();
  frBox box(gtl::xl(markerRect), gtl::yl(markerRect), gtl::xh(markerRect), gtl::yh(markerRect));
  marker->setBBox(box);
  marker->setLayerNum(layerNum);
  marker->setConstraint(getDesign()->getTech()->getLayer(layerNum)->getMinSpacing());
  marker->addSrc(net1->getOwner());
  marker->addSrc(net2->getOwner());
  if (addMarker(marker)) {
    // true marker
    if (enableOutput) {
      double dbu = getDesign()->getTopBlock()->getDBUPerUU();
      cout <<"MetSpc@(" <<gtl::xl(markerRect) / dbu <<", " <<gtl::yl(markerRect) / dbu <<") ("
                        <<gtl::xh(markerRect) / dbu <<", " <<gtl::yh(markerRect) / dbu <<") "
           <<getDesign()->getTech()->getLayer(layerNum)->getName() <<" ";
      auto owner = net1->getOwner();
      if (owner == nullptr) {
        cout <<"FLOATING";
      } else {
        if (owner->typeId() == frcNet) {
          cout <<static_cast<frNet*>(owner)->getName();
        } else if (owner->typeId() == frcInstTerm) {
          cout <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
               <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
        } else if (owner->typeId() == frcTerm) {
          cout <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
        } else if (owner->typeId() == frcInstBlockage) {
          cout <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
        } else if (owner->typeId() == frcBlockage) {
          cout <<"PIN/OBS";
        } else {
          cout <<"UNKNOWN";
        }
      }
      cout <<" ";
      owner = net2->getOwner();
      if (owner == nullptr) {
        cout <<"FLOATING";
      } else {
        if (owner->typeId() == frcNet) {
          cout <<static_cast<frNet*>(owner)->getName();
        } else if (owner->typeId() == frcInstTerm) {
          cout <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
               <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
        } else if (owner->typeId() == frcTerm) {
          cout <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
        } else if (owner->typeId() == frcInstBlockage) {
          cout <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
        } else if (owner->typeId() == frcBlockage) {
          cout <<"PIN/OBS";
        } else {
          cout <<"UNKNOWN";
        }
      }
      cout <<endl;
    }
  }
}

bool FlexGCWorker::checkMetalSpacing_short_skipOBSPin(gcRect* rect1, gcRect* rect2, const gtl::rectangle_data<frCoord> &markerRect) {
  bool isRect1Obs = false;
  bool isRect2Obs = false;
  if (rect1->getNet()->getOwner() && 
      (rect1->getNet()->getOwner()->typeId() == frcInstBlockage ||
       rect1->getNet()->getOwner()->typeId() == frcBlockage)) {
    isRect1Obs = true;
  }
  if (rect2->getNet()->getOwner() && 
      (rect2->getNet()->getOwner()->typeId() == frcInstBlockage ||
       rect2->getNet()->getOwner()->typeId() == frcBlockage)) {
    isRect2Obs = true;
  }
  if (!isRect1Obs && !isRect2Obs) {
    return false;
  }
  if (isRect1Obs && isRect2Obs) {
    return false;
  }
  // always make obs to be rect2
  if (isRect1Obs) {
    std::swap(rect1, rect2);
  }
  // now rect is not obs, rect2 is obs
  auto layerNum = rect1->getLayerNum();
  auto net1 = rect1->getNet();

  // check if markerRect is covered by fixed shapes of net1
  auto &polys1 = net1->getPolygons(layerNum, true);
  vector<gtl::rectangle_data<frCoord> > rects;
  gtl::get_max_rectangles(rects, polys1);
  for (auto &rect: rects) {
    if (gtl::contains(rect, markerRect)) {
      return true;
    }
  }
  return false;
}

void FlexGCWorker::checkMetalSpacing_short(gcRect* rect1, gcRect* rect2, const gtl::rectangle_data<frCoord> &markerRect) {
  bool enableOutput = printMarker;

  auto layerNum = rect1->getLayerNum();
  auto net1 = rect1->getNet();
  auto net2 = rect2->getNet();
  bool testflag = false;
  //bool testflag = true;
  //if (layerNum == 0 &&
  //    gtl::xl(markerRect) == frCoord(335.955 * 2000) &&
  //    gtl::yl(markerRect) == frCoord(134.500 * 2000) &&
  //    gtl::xh(markerRect) == frCoord(336.005 * 2000) &&
  //    gtl::yh(markerRect) == frCoord(134.545 * 2000)) {
  //  testflag = true;
  //  cout <<"@@@0" <<endl;
  //}
  //210.09, 88.89 ) ( 210.145, 88.945
  //if (layerNum == 0 &&
  //    gtl::xl(markerRect) == frCoord(210.090 * 2000) &&
  //    gtl::yl(markerRect) == frCoord( 88.890 * 2000) &&
  //    gtl::xh(markerRect) == frCoord(210.145 * 2000) &&
  //    gtl::yh(markerRect) == frCoord( 88.945 * 2000)) {
  //  testflag = true;
  //  cout <<"@@@0" <<endl;
  //}
  /// skip fixed shape
  if (rect1->isFixed() && rect2->isFixed()) {
    if (testflag) {
      cout <<"@@@1" <<endl;
    }
    return;
  }
  // skip obs overlaps with pin
  if (checkMetalSpacing_short_skipOBSPin(rect1, rect2, markerRect)) {
    return;
  }

  // skip if marker area does not have route shape, must exclude touching
  {
    // bloat marker by minimum coord if zero
    gtl::rectangle_data<frCoord> bloatMarkerRect(markerRect);
    if (gtl::delta(markerRect, gtl::HORIZONTAL) == 0) {
      gtl::bloat(bloatMarkerRect, gtl::HORIZONTAL, 1);
    }
    if (gtl::delta(markerRect, gtl::VERTICAL) == 0) {
      gtl::bloat(bloatMarkerRect, gtl::VERTICAL, 1);
    }
    using namespace boost::polygon::operators;
    auto &polys1 = net1->getPolygons(layerNum, false);
    auto intersection_polys1 = polys1 & bloatMarkerRect;
    auto &polys2 = net2->getPolygons(layerNum, false);
    auto intersection_polys2 = polys2 & bloatMarkerRect;
    if (gtl::empty(intersection_polys1) && gtl::empty(intersection_polys2)) {
      if (testflag) {
        cout <<"@@@2" <<endl;
      }
      return;
    }
  }
  // skip same-net sufficient metal
  if (net1 == net2) {
    // skip if good
    auto minWidth = getDesign()->getTech()->getLayer(layerNum)->getMinWidth();
    auto xLen = gtl::delta(markerRect, gtl::HORIZONTAL);
    auto yLen = gtl::delta(markerRect, gtl::VERTICAL);
    if (xLen * xLen + yLen * yLen >= minWidth * minWidth) {
      if (testflag) {
        cout <<"@@@3" <<endl;
      }
      return;
    }
    // skip if rect < minwidth
    if ((gtl::delta(*rect1, gtl::HORIZONTAL) < minWidth || gtl::delta(*rect1, gtl::VERTICAL) < minWidth) ||
        (gtl::delta(*rect2, gtl::HORIZONTAL) < minWidth || gtl::delta(*rect2, gtl::VERTICAL) < minWidth)) {
      if (testflag) {
        cout <<"@@@4" <<endl;
      }
      return;
    }
    // query third object that can bridge rect1 and rect2 in bloated marker area
    gtl::point_data<frCoord> centerPt;
    gtl::center(centerPt, markerRect);
    gtl::rectangle_data<frCoord> bloatMarkerRect(centerPt.x(), centerPt.y(), centerPt.x(), centerPt.y());
    box_t queryBox;
    myBloat(bloatMarkerRect, minWidth, queryBox);

    auto &workerRegionQuery = getWorkerRegionQuery();
    vector<rq_rptr_value_t<gcRect> > result;
    workerRegionQuery.queryMaxRectangle(queryBox, layerNum, result);
    //cout <<"3rd obj" <<endl;
    for (auto &[objBox, objPtr]: result) {
      if (objPtr == rect1 || objPtr == rect2) {
        continue;
      }
      if (objPtr->getNet() != net1) {
        continue;
      }
      if (!gtl::contains(*objPtr, markerRect)) {
        continue;
      }
      if (gtl::delta(*objPtr, gtl::HORIZONTAL) < minWidth || gtl::delta(*objPtr, gtl::VERTICAL) < minWidth) {
        continue;
      }
      //cout <<"@@@x (" 
      //     <<gtl::xl(*objPtr) / 2000.0 <<", "
      //     <<gtl::yl(*objPtr) / 2000.0 <<") ("
      //     <<gtl::xh(*objPtr) / 2000.0 <<", "
      //     <<gtl::yh(*objPtr) / 2000.0 <<") ("
      //     <<endl;
      // only check same net third object
      gtl::rectangle_data<frCoord> tmpRect1(*rect1);
      gtl::rectangle_data<frCoord> tmpRect2(*rect2);
      if (gtl::intersect(tmpRect1, *objPtr) && gtl::intersect(tmpRect2, *objPtr)) {
        //if (gtl::intersect(tmpRect1, tmpRect2)) {
        //  if (gtl::contains(tmpRect1, markerRect)) {
        //    auto xLen1 = gtl::delta(tmpRect1, gtl::HORIZONTAL);
        //    auto yLen1 = gtl::delta(tmpRect1, gtl::VERTICAL);
        //    if (xLen1 * xLen1 + yLen1 * yLen1 >= minWidth * minWidth) {
        //      if (testflag) {
        //        cout <<"@@@5 (" 
        //             <<gtl::xl(tmpRect1) / 2000.0 <<", "
        //             <<gtl::yl(tmpRect1) / 2000.0 <<") ("
        //             <<gtl::xh(tmpRect1) / 2000.0 <<", "
        //             <<gtl::yh(tmpRect1) / 2000.0 <<") ("
        //             <<endl;
        //      }
        //      return;
        //    }
        //  }
        //}
        auto xLen1 = gtl::delta(tmpRect1, gtl::HORIZONTAL);
        auto yLen1 = gtl::delta(tmpRect1, gtl::VERTICAL);
        auto xLen2 = gtl::delta(tmpRect2, gtl::HORIZONTAL);
        auto yLen2 = gtl::delta(tmpRect2, gtl::VERTICAL);
        //cout <<"@@@x5 (" 
        //     <<gtl::xl(tmpRect1) / 2000.0 <<", "
        //     <<gtl::yl(tmpRect1) / 2000.0 <<") ("
        //     <<gtl::xh(tmpRect1) / 2000.0 <<", "
        //     <<gtl::yh(tmpRect1) / 2000.0 <<") ("
        //     <<gtl::xl(tmpRect2) / 2000.0 <<", "
        //     <<gtl::yl(tmpRect2) / 2000.0 <<") ("
        //     <<gtl::xh(tmpRect2) / 2000.0 <<", "
        //     <<gtl::yh(tmpRect2) / 2000.0 <<")"
        //     <<endl;
        if (xLen1 * xLen1 + yLen1 * yLen1 >= minWidth * minWidth &&
            xLen2 * xLen2 + yLen2 * yLen2 >= minWidth * minWidth) {
          //cout <<"@@@x5 returned" <<endl;
          if (testflag) {
            cout <<"@@@5 (" 
                 <<gtl::xl(tmpRect1) / 2000.0 <<", "
                 <<gtl::yl(tmpRect1) / 2000.0 <<") ("
                 <<gtl::xh(tmpRect1) / 2000.0 <<", "
                 <<gtl::yh(tmpRect1) / 2000.0 <<") ("
                 <<gtl::xl(tmpRect2) / 2000.0 <<", "
                 <<gtl::yl(tmpRect2) / 2000.0 <<") ("
                 <<gtl::xh(tmpRect2) / 2000.0 <<", "
                 <<gtl::yh(tmpRect2) / 2000.0 <<")"
                 <<endl;
          }
          return;
        }
      }
    }
  }

  auto marker = make_unique<frMarker>();
  frBox box(gtl::xl(markerRect), gtl::yl(markerRect), gtl::xh(markerRect), gtl::yh(markerRect));
  marker->setBBox(box);
  marker->setLayerNum(layerNum);
  if (net1 == net2) {
    marker->setConstraint(getDesign()->getTech()->getLayer(layerNum)->getNonSufficientMetalConstraint());
  } else {
    marker->setConstraint(getDesign()->getTech()->getLayer(layerNum)->getShortConstraint());
  }
  marker->addSrc(net1->getOwner());
  marker->addSrc(net2->getOwner());
  if (addMarker(marker)) {
    // true marker
    if (enableOutput) {
      double dbu = getDesign()->getTopBlock()->getDBUPerUU();
      //cout <<"@@@("
      //     <<gtl::xl(*rect1) / dbu <<", " <<gtl::yl(*rect1) / dbu <<") ("
      //     <<gtl::xh(*rect1) / dbu <<", " <<gtl::yh(*rect1) / dbu <<") ("
      //     <<gtl::xl(*rect2) / dbu <<", " <<gtl::yl(*rect2) / dbu <<") ("
      //     <<gtl::xh(*rect2) / dbu <<", " <<gtl::yh(*rect2) / dbu <<") "
      //     <<endl;
      if (net1 == net2) {
        cout <<"NSMetal@(";
      } else {
        cout <<"Short@(";
      }
      cout <<gtl::xl(markerRect) / dbu <<", " <<gtl::yl(markerRect) / dbu <<") ("
           <<gtl::xh(markerRect) / dbu <<", " <<gtl::yh(markerRect) / dbu <<") "
           <<getDesign()->getTech()->getLayer(layerNum)->getName() <<" ";
      auto owner = net1->getOwner();
      if (owner == nullptr) {
        cout <<"FLOATING";
      } else {
        if (owner->typeId() == frcNet) {
          cout <<static_cast<frNet*>(owner)->getName();
        } else if (owner->typeId() == frcInstTerm) {
          cout <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
               <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
        } else if (owner->typeId() == frcTerm) {
          cout <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
        } else if (owner->typeId() == frcInstBlockage) {
          cout <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
        } else if (owner->typeId() == frcBlockage) {
          cout <<"PIN/OBS";
        } else {
          cout <<"UNKNOWN";
        }
      }
      cout <<" ";
      owner = net2->getOwner();
      if (owner == nullptr) {
        cout <<"FLOATING";
      } else {
        if (owner->typeId() == frcNet) {
          cout <<static_cast<frNet*>(owner)->getName();
        } else if (owner->typeId() == frcInstTerm) {
          cout <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
               <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
        } else if (owner->typeId() == frcTerm) {
          cout <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
        } else if (owner->typeId() == frcInstBlockage) {
          cout <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
        } else if (owner->typeId() == frcBlockage) {
          cout <<"PIN/OBS";
        } else {
          cout <<"UNKNOWN";
        }
      }
      cout <<endl;
    }
  }
}

void FlexGCWorker::checkMetalSpacing_main(gcRect* ptr1, gcRect* ptr2) {
  //bool enableOutput = true;

  // NSMetal does not need self-intersection
  // Minimum width rule handles outsite this function
  if (ptr1 == ptr2) {
    return;
  }
  //auto layerNum = ptr1->getLayerNum();
  //auto net1 = ptr1->getNet();
  //auto net2 = ptr2->getNet();

  gtl::rectangle_data<frCoord> markerRect(*ptr1);
  auto distX = gtl::euclidean_distance(markerRect, *ptr2, gtl::HORIZONTAL);
  auto distY = gtl::euclidean_distance(markerRect, *ptr2, gtl::VERTICAL);

  gtl::generalized_intersect(markerRect, *ptr2);
  auto prlX = gtl::delta(markerRect, gtl::HORIZONTAL);
  auto prlY = gtl::delta(markerRect, gtl::VERTICAL);

  if (distX) {
    prlX = -prlX;
  }
  if (distY) {
    prlY = -prlY;
  }
  //// x spacing
  //if (distX > 0 && distY == 0) {
  //  checkMetalPrl_prl(ptr1, ptr2, markerRect, prlY, distX * distX);
  //// y spacing
  //} else if (distX == 0 && distY > 0) {
  //  checkMetalPrl_prl(ptr1, ptr2, markerRect, prlX, distY * distY);
  //// diag spacing
  //} else if (distX > 0 && distY > 0) {
  //  checkMetalPrl_prl(ptr1, ptr2, markerRect, std::max(prlX, prlY), distX * distX + distY * distY);
  //// short, nsmetal
  //} else {
  //  ;
  //}
  
  // short, nsmetal
  if (distX == 0 && distY == 0) {
    checkMetalSpacing_short(ptr1, ptr2, markerRect);
  // prl
  } else {
    checkMetalSpacing_prl(ptr1, ptr2, markerRect, std::max(prlX, prlY), distX, distY);
  }
}



void FlexGCWorker::checkMetalSpacing_main(gcRect* rect) {
  //bool enableOutput = true;
  bool enableOutput = false;

  auto layerNum = rect->getLayerNum();
  auto maxSpcVal = checkMetalSpacing_getMaxSpcVal(layerNum);
  box_t queryBox;
  myBloat(*rect, maxSpcVal, queryBox);
  if (enableOutput) {
    double dbu = getDesign()->getTopBlock()->getDBUPerUU();
    cout <<"checkMetalPrl maxRect ";
    if (rect->isFixed()) {
      cout <<"FIXED";
    } else {
      cout <<"ROUTE";
    }
    cout <<" (" <<gtl::xl(*rect) / dbu <<", " <<gtl::yl(*rect) / dbu <<") (" 
                <<gtl::xh(*rect) / dbu <<", " <<gtl::yh(*rect) / dbu <<") "
         <<getDesign()->getTech()->getLayer(layerNum)->getName() <<" ";
    cout <<"bloat maxSpcVal@" <<maxSpcVal / dbu <<" (" <<queryBox.min_corner().x() / dbu <<", " <<queryBox.min_corner().x() / dbu <<") (" 
                      <<queryBox.max_corner().x() / dbu <<", " <<queryBox.max_corner().x() / dbu <<") ";
    auto owner = rect->getNet()->getOwner();
    if (owner == nullptr) {
      cout <<" FLOATING";
    } else {
      if (owner->typeId() == frcNet) {
        cout <<static_cast<frNet*>(owner)->getName();
      } else if (owner->typeId() == frcInstTerm) {
        cout <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
             <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
      } else if (owner->typeId() == frcTerm) {
        cout <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
      } else if (owner->typeId() == frcInstBlockage) {
        cout <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
      } else if (owner->typeId() == frcBlockage) {
        cout <<"PIN/OBS";
      } else {
        cout <<"UNKNOWN";
      }
    }
    cout <<endl;
  }

  auto &workerRegionQuery = getWorkerRegionQuery();
  vector<rq_rptr_value_t<gcRect> > result;
  workerRegionQuery.queryMaxRectangle(queryBox, layerNum, result);
  // Short, metSpc, NSMetal here
  for (auto &[objBox, ptr]: result) {
    //cout <<"from " <<rect <<" to " <<ptr <<endl;
    checkMetalSpacing_main(rect, ptr);
  }
}

void FlexGCWorker::checkMetalSpacing() {
  if (targetNet) {
    // layer --> net --> polygon --> maxrect
    for (int i = std::max((frLayerNum)(getDesign()->getTech()->getBottomLayerNum()), minLayerNum); 
         i <= std::min((frLayerNum)(getDesign()->getTech()->getTopLayerNum()), maxLayerNum); i++) {
      auto currLayer = getDesign()->getTech()->getLayer(i);
      if (currLayer->getType() != frLayerTypeEnum::ROUTING) {
        continue;
      }
      for (auto &pin: targetNet->getPins(i)) {
        for (auto &maxrect: pin->getMaxRectangles()) {
          checkMetalSpacing_main(maxrect.get());
        }
      }
    }
  } else {
    // layer --> net --> polygon --> maxrect
    for (int i = std::max((frLayerNum)(getDesign()->getTech()->getBottomLayerNum()), minLayerNum); 
         i <= std::min((frLayerNum)(getDesign()->getTech()->getTopLayerNum()), maxLayerNum); i++) {
      auto currLayer = getDesign()->getTech()->getLayer(i);
      if (currLayer->getType() != frLayerTypeEnum::ROUTING) {
        continue;
      }
      for (auto &net: getNets()) {
        for (auto &pin: net->getPins(i)) {
          for (auto &maxrect: pin->getMaxRectangles()) {
            // Short, NSMetal, metSpc
            //cout <<"from " <<maxrect.get() <<endl;
            checkMetalSpacing_main(maxrect.get());
          }
        }
      }
    }
  }
}

void FlexGCWorker::checkMetalShape_minWidth(const gtl::rectangle_data<frCoord> &rect, frLayerNum layerNum, gcNet* net, bool isH) {
  bool enableOutput = printMarker;
  // skip enough width
  auto minWidth = getDesign()->getTech()->getLayer(layerNum)->getMinWidth();
  auto xLen = gtl::delta(rect, gtl::HORIZONTAL);
  auto yLen = gtl::delta(rect, gtl::VERTICAL);
  if (isH && xLen >= minWidth) {
    return;
  }
  if (!isH && yLen >= minWidth) {
    return;
  }
  // only show marker if fixed area < marker area
  {
    using namespace boost::polygon::operators;
    auto &fixedPolys = net->getPolygons(layerNum, true);
    auto intersection_fixedPolys = fixedPolys & rect;
    if (gtl::area(intersection_fixedPolys) == gtl::area(rect)) {
      return;
    }
  }
  
  auto marker = make_unique<frMarker>();
  frBox box(gtl::xl(rect), gtl::yl(rect), gtl::xh(rect), gtl::yh(rect));
  marker->setBBox(box);
  marker->setLayerNum(layerNum);
  marker->setConstraint(getDesign()->getTech()->getLayer(layerNum)->getMinWidthConstraint());
  marker->addSrc(net->getOwner());
  if (addMarker(marker)) {
    // true marker
    if (enableOutput) {
      double dbu = getDesign()->getTopBlock()->getDBUPerUU();
      cout <<"MinWid@(";
      cout <<gtl::xl(rect) / dbu <<", " <<gtl::yl(rect) / dbu <<") ("
           <<gtl::xh(rect) / dbu <<", " <<gtl::yh(rect) / dbu <<") "
           <<getDesign()->getTech()->getLayer(layerNum)->getName() <<" ";
      auto owner = net->getOwner();
      if (owner == nullptr) {
        cout <<"FLOATING";
      } else {
        if (owner->typeId() == frcNet) {
          cout <<static_cast<frNet*>(owner)->getName();
        } else if (owner->typeId() == frcInstTerm) {
          cout <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
               <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
        } else if (owner->typeId() == frcTerm) {
          cout <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
        } else if (owner->typeId() == frcInstBlockage) {
          cout <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
        } else if (owner->typeId() == frcBlockage) {
          cout <<"PIN/OBS";
        } else {
          cout <<"UNKNOWN";
        }
      }
      cout <<endl;
    }
  }
}

void FlexGCWorker::checkMetalShape_minStep_helper(const frBox &markerBox, frLayerNum layerNum, gcNet* net, 
                                                  frMinStepConstraint* con,
                                                  bool hasInsideCorner, bool hasOutsideCorner, bool hasStep, 
                                                  int currEdges, frCoord currLength, bool hasRoute) {
  bool enableOutput = printMarker;
  // skip if no edge
  if (currEdges == 0) {
    return;
  }
  // skip if no route
  if (!hasRoute) {
    return;
  }

  if (con->hasMinstepType()) {
    // skip if no corner or step
    switch(con->getMinstepType()) {
      case frMinstepTypeEnum::INSIDECORNER:
        if (!hasInsideCorner) {
          return;
        }
        break;
      case frMinstepTypeEnum::OUTSIDECORNER:
        if (!hasOutsideCorner) {
          return;
        }
        break;
      case frMinstepTypeEnum::STEP:
        if (!hasStep) {
          return;
        }
        break;
      default:
        ;
    }
    // skip if <= maxlength
    if (currLength <= con->getMaxLength()) {
      return;
    }
  } else if (con->hasMaxEdges()) {
    // skip if <= maxedges
    if (currEdges <= con->getMaxEdges()) {
      return;
    }
  }

  // true marker
  auto marker = make_unique<frMarker>();
  marker->setBBox(markerBox);
  marker->setLayerNum(layerNum);
  marker->setConstraint(con);
  marker->addSrc(net->getOwner());
  if (addMarker(marker)) {
    // true marker
    if (enableOutput) {
      double dbu = getDesign()->getTopBlock()->getDBUPerUU();
      cout <<"MinStp@(";
      cout <<markerBox.left()  / dbu <<", " <<markerBox.bottom() / dbu <<") ("
           <<markerBox.right() / dbu <<", " <<markerBox.top()    / dbu <<") "
           <<getDesign()->getTech()->getLayer(layerNum)->getName() <<" ";
      auto owner = net->getOwner();
      if (owner == nullptr) {
        cout <<"FLOATING";
      } else {
        if (owner->typeId() == frcNet) {
          cout <<static_cast<frNet*>(owner)->getName();
        } else if (owner->typeId() == frcInstTerm) {
          cout <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
               <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
        } else if (owner->typeId() == frcTerm) {
          cout <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
        } else if (owner->typeId() == frcInstBlockage) {
          cout <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
        } else if (owner->typeId() == frcBlockage) {
          cout <<"PIN/OBS";
        } else {
          cout <<"UNKNOWN";
        }
      }
      cout <<endl;
    }
  }
}

void FlexGCWorker::checkMetalShape_minStep(gcPin* pin) {
  //bool enableOutput = true;

  auto poly = pin->getPolygon();
  auto layerNum = poly->getLayerNum();
  auto net = poly->getNet();

  auto con = design->getTech()->getLayer(layerNum)->getMinStepConstraint();

  if (!con) {
    return;
  }

  gcSegment* be = nullptr;
  gcSegment* firste = nullptr; // first edge to check
  int currEdges = 0;
  int currLength = 0;
  bool hasRoute = false;
  frCoord llx = 0;
  frCoord lly = 0;
  frCoord urx = 0;
  frCoord ury = 0;
  frBox markerBox;
  bool hasInsideCorner = false;
  bool hasOutsideCorner = false;
  bool hasStep = false;
  auto minStepLength = con->getMinStepLength();
  for (auto &edges: pin->getPolygonEdges()) {
    // get the first edge that is >= minstep length
    firste = nullptr;
    for (auto &e: edges) {
      if (gtl::length(*e) >= minStepLength) {
        firste = e.get();
        break;
      }
    }
    // skip if no first starting edge
    if (!firste) {
      continue;
    }
    // initialize all vars
    auto edge = firste;
    be = edge;
    currEdges = 0;
    currLength = 0;
    hasRoute = edge->isFixed() ? false : true;
    hasInsideCorner = false;
    hasOutsideCorner = false;
    hasStep = false;
    llx = edge->high().x();
    lly = edge->high().y();
    urx = edge->high().x();
    ury = edge->high().y();
    while (1) {
      edge = edge->getNextEdge();
      if (gtl::length(*edge) < minStepLength) {
        currEdges++;
        currLength += gtl::length(*edge);
        hasRoute = hasRoute || (edge->isFixed() ? false : true);
        llx = std::min(llx, edge->high().x());
        lly = std::min(lly, edge->high().y());
        urx = std::max(urx, edge->high().x());
        ury = std::max(ury, edge->high().y());
      } else {
        // skip if begin end edges are the same
        if (edge == be) {
          break;
        }
        // be and ee found, check rule here
        hasRoute = hasRoute || (edge->isFixed() ? false : true);
        markerBox.set(llx, lly, urx, ury);
        checkMetalShape_minStep_helper(markerBox, layerNum, net, con, hasInsideCorner, hasOutsideCorner, hasStep, 
                                       currEdges, currLength, hasRoute);
        be = edge; // new begin edge
        // skip if new begin edge is the first begin edge
        if (be == firste) {
          break;
        }
        currEdges = 0;
        currLength = 0;
        hasRoute = edge->isFixed() ? false : true;
        hasInsideCorner = false;
        hasOutsideCorner = false;
        hasStep = false;
        llx = edge->high().x();
        lly = edge->high().y();
        urx = edge->high().x();
        ury = edge->high().y();
      }
    }
  }
}

void FlexGCWorker::checkMetalShape_offGrid(gcPin* pin) {
  bool enableOutput = false;
  auto net = pin->getNet();
  auto mGrid = getDesign()->getTech()->getManufacturingGrid();
  for (auto &rect: pin->getMaxRectangles()) {
    auto maxRect = rect.get();
    auto layerNum = maxRect->getLayerNum();
    // off grid maxRect
    if (gtl::xl(*maxRect) % mGrid || 
        gtl::xh(*maxRect) % mGrid ||
        gtl::yl(*maxRect) % mGrid ||
        gtl::yh(*maxRect) % mGrid) {
      // continue if the marker area does not have route shape
      auto &polys = net->getPolygons(layerNum, false);
      gtl::rectangle_data<frCoord> markerRect(*maxRect);
      using namespace boost::polygon::operators;
      auto intersection_polys = polys & markerRect;
      if (gtl::empty(intersection_polys)) {
        continue;
      }
      auto marker = make_unique<frMarker>();
      frBox box(gtl::xl(markerRect), gtl::yl(markerRect), gtl::xh(markerRect), gtl::yh(markerRect));
      marker->setBBox(box);
      marker->setLayerNum(layerNum);
      marker->setConstraint(getDesign()->getTech()->getLayer(layerNum)->getOffGridConstraint());
      marker->addSrc(net->getOwner());
      if (addMarker(marker)) {
        if (enableOutput) {
          double dbu = getDesign()->getTopBlock()->getDBUPerUU();
          
          cout << "OffGrid@(";
          cout <<gtl::xl(markerRect) / dbu <<", " <<gtl::yl(markerRect) / dbu <<") ("
               <<gtl::xh(markerRect) / dbu <<", " <<gtl::yh(markerRect) / dbu <<") "
               <<getDesign()->getTech()->getLayer(layerNum)->getName() <<" ";
          auto owner = net->getOwner();
          if (owner == nullptr) {
            cout <<"FLOATING";
          } else {
            if (owner->typeId() == frcNet) {
              cout <<static_cast<frNet*>(owner)->getName();
            } else if (owner->typeId() == frcInstTerm) {
              cout <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
                   <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
            } else if (owner->typeId() == frcTerm) {
              cout <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
            } else if (owner->typeId() == frcInstBlockage) {
              cout <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
            } else if (owner->typeId() == frcBlockage) {
              cout <<"PIN/OBS";
            } else {
              cout <<"UNKNOWN";
            }
          }
          cout << endl;
        }
      }
    }
  }
}

void FlexGCWorker::checkMetalShape_minEnclosedArea(gcPin* pin) {
  bool enableOutput = false;
  auto net = pin->getNet();
  auto poly = pin->getPolygon();
  auto layerNum = poly->getLayerNum();
  if (getDesign()->getTech()->getLayer(layerNum)->hasMinEnclosedArea()) {
    for (auto holeIt = poly->begin_holes(); holeIt != poly->end_holes(); holeIt++) {
      auto &hole_poly = *holeIt;
      for (auto con: getDesign()->getTech()->getLayer(layerNum)->getMinEnclosedAreaConstraints()) {
        auto reqArea = con->getArea();
        if (gtl::area(hole_poly) < reqArea) {
          auto &polys = net->getPolygons(layerNum, false);
          using namespace boost::polygon::operators;
          auto intersection_polys = polys & (*poly);
          if (gtl::empty(intersection_polys)) {
            continue;
          }
          // create marker
          gtl::rectangle_data<frCoord> markerRect;
          gtl::extents(markerRect, hole_poly);

          auto marker = make_unique<frMarker>();
          frBox box(gtl::xl(markerRect), gtl::yl(markerRect), gtl::xh(markerRect), gtl::yh(markerRect));
          marker->setBBox(box);
          marker->setLayerNum(layerNum);
          marker->setConstraint(con);
          marker->addSrc(net->getOwner());
          if (addMarker(marker)) {
            if (enableOutput) {
              double dbu = getDesign()->getTopBlock()->getDBUPerUU();
              
              cout << "MinHole@(";
              cout <<gtl::xl(markerRect) / dbu <<", " <<gtl::yl(markerRect) / dbu <<") ("
                   <<gtl::xh(markerRect) / dbu <<", " <<gtl::yh(markerRect) / dbu <<") "
                   <<getDesign()->getTech()->getLayer(layerNum)->getName() <<" ";
              auto owner = net->getOwner();
              if (owner == nullptr) {
                cout <<"FLOATING";
              } else {
                if (owner->typeId() == frcNet) {
                  cout <<static_cast<frNet*>(owner)->getName();
                } else if (owner->typeId() == frcInstTerm) {
                  cout <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
                       <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
                } else if (owner->typeId() == frcTerm) {
                  cout <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
                } else if (owner->typeId() == frcInstBlockage) {
                  cout <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
                } else if (owner->typeId() == frcBlockage) {
                  cout <<"PIN/OBS";
                } else {
                  cout <<"UNKNOWN";
                }
              }
              cout << endl;
            }
          }
        }
      }
    }
  }
}

void FlexGCWorker::checkMetalShape_main(gcPin* pin) {
  //bool enableOutput = true;

  auto poly = pin->getPolygon();
  auto layerNum = poly->getLayerNum();
  auto net = poly->getNet();

  // min width
  vector<gtl::rectangle_data<frCoord> > rects;
  gtl::polygon_90_set_data<frCoord> polySet;
  {
    using namespace boost::polygon::operators;
    polySet += *poly;
  }
  polySet.get_rectangles(rects, gtl::HORIZONTAL);
  for (auto &rect: rects) {
    checkMetalShape_minWidth(rect, layerNum, net, true);
  }
  rects.clear();
  polySet.get_rectangles(rects, gtl::VERTICAL);
  for (auto &rect: rects) {
    checkMetalShape_minWidth(rect, layerNum, net, false);
  }

  // min step
  checkMetalShape_minStep(pin);

  // off grid
  checkMetalShape_offGrid(pin);

  // min hole
  checkMetalShape_minEnclosedArea(pin);
}

void FlexGCWorker::checkMetalShape() {
  if (targetNet) {
    // layer --> net --> polygon
    for (int i = std::max((frLayerNum)(getDesign()->getTech()->getBottomLayerNum()), minLayerNum); 
         i <= std::min((frLayerNum)(getDesign()->getTech()->getTopLayerNum()), maxLayerNum); i++) {
      auto currLayer = getDesign()->getTech()->getLayer(i);
      if (currLayer->getType() != frLayerTypeEnum::ROUTING) {
        continue;
      }
      for (auto &pin: targetNet->getPins(i)) {
        checkMetalShape_main(pin.get());
      }
    }
  } else {
    // layer --> net --> polygon
    for (int i = std::max((frLayerNum)(getDesign()->getTech()->getBottomLayerNum()), minLayerNum); 
         i <= std::min((frLayerNum)(getDesign()->getTech()->getTopLayerNum()), maxLayerNum); i++) {
      auto currLayer = getDesign()->getTech()->getLayer(i);
      if (currLayer->getType() != frLayerTypeEnum::ROUTING) {
        continue;
      }
      for (auto &net: getNets()) {
        for (auto &pin: net->getPins(i)) {
          checkMetalShape_main(pin.get());
        }
      }
    }
  }
}

bool FlexGCWorker::checkMetalEndOfLine_eol_isEolEdge(gcSegment *edge, frSpacingEndOfLineConstraint *con) {
  // skip if >= eolWidth
  if (gtl::length(*edge) >= con->getEolWidth()) {
    return false;
  }
  // skip if non convex edge
  auto prevEdge = edge->getPrevEdge();
  auto nextEdge = edge->getNextEdge();
  // using gtl default generation of hole polygon from polyset, 1 means convex
  //if (gtl::orientation(*prevEdge, *edge) == 1 && gtl::orientation(*edge, *nextEdge) == 1) {
  //  double dbu = getDesign()->getTopBlock()->getDBUPerUU();
  //  cout <<"both 1 (" <<prevEdge->low().x()  / dbu <<", " <<prevEdge->low().y()  / dbu <<") --> ("
  //                    <<edge->low().x()      / dbu <<", " <<edge->low().y()      / dbu <<") --> ("
  //                    <<nextEdge->low().x()  / dbu <<", " <<nextEdge->low().y()  / dbu <<") --> ("
  //                    <<nextEdge->high().x() / dbu <<", " <<nextEdge->high().y() / dbu <<") "
  //                    <<getDesign()->getTech()->getLayer(edge->getLayerNum())->getName() <<endl;
  //}
  //if (gtl::orientation(*prevEdge, *edge) == -1 && gtl::orientation(*edge, *nextEdge) == -1) {
  //  double dbu = getDesign()->getTopBlock()->getDBUPerUU();
  //  cout <<"both -1 (" <<prevEdge->low().x() / dbu <<", " <<prevEdge->low().y()  / dbu <<") --> ("
  //                    <<edge->low().x()      / dbu <<", " <<edge->low().y()      / dbu <<") --> ("
  //                    <<nextEdge->low().x()  / dbu <<", " <<nextEdge->low().y()  / dbu <<") --> ("
  //                    <<nextEdge->high().x() / dbu <<", " <<nextEdge->high().y() / dbu <<") "
  //                    <<getDesign()->getTech()->getLayer(edge->getLayerNum())->getName() <<endl;
  //}
  if (!(gtl::orientation(*prevEdge, *edge) == 1 && gtl::orientation(*edge, *nextEdge) == 1)) {
    return false;
  }
  return true;
}

// bbox on the gcSegment->low() side
void FlexGCWorker::checkMetalEndOfLine_eol_hasParallelEdge_oneDir_getQueryBox(gcSegment *edge, frSpacingEndOfLineConstraint *con, 
                                                                              bool isSegLow, box_t &queryBox, 
                                                                              gtl::rectangle_data<frCoord> &queryRect) {
  frCoord ptX, ptY;
  auto eolWithin = con->getEolWithin();
  auto parWithin = con->getParWithin();
  auto parSpace  = con->getParSpace();
  if (isSegLow) {
    ptX = edge->low().x();
    ptY = edge->low().y();
    if (edge->getDir() == frDirEnum::E) {
      bg::set<bg::min_corner, 0>(queryBox, ptX - parSpace  );
      bg::set<bg::min_corner, 1>(queryBox, ptY - eolWithin );
      bg::set<bg::max_corner, 0>(queryBox, ptX             );
      bg::set<bg::max_corner, 1>(queryBox, ptY + parWithin );
    } else if (edge->getDir() == frDirEnum::W) {
      bg::set<bg::min_corner, 0>(queryBox, ptX             );
      bg::set<bg::min_corner, 1>(queryBox, ptY - parWithin );
      bg::set<bg::max_corner, 0>(queryBox, ptX + parSpace  );
      bg::set<bg::max_corner, 1>(queryBox, ptY + eolWithin );
    } else if (edge->getDir() == frDirEnum::N) {
      bg::set<bg::min_corner, 0>(queryBox, ptX - parWithin );
      bg::set<bg::min_corner, 1>(queryBox, ptY - parSpace  );
      bg::set<bg::max_corner, 0>(queryBox, ptX + eolWithin );
      bg::set<bg::max_corner, 1>(queryBox, ptY             );
    } else { // S
      bg::set<bg::min_corner, 0>(queryBox, ptX - eolWithin );
      bg::set<bg::min_corner, 1>(queryBox, ptY             );
      bg::set<bg::max_corner, 0>(queryBox, ptX + parWithin );
      bg::set<bg::max_corner, 1>(queryBox, ptY + parSpace  );
    }
  } else {
    ptX = edge->high().x();
    ptY = edge->high().y();
    if (edge->getDir() == frDirEnum::E) {
      bg::set<bg::min_corner, 0>(queryBox, ptX             );
      bg::set<bg::min_corner, 1>(queryBox, ptY - eolWithin );
      bg::set<bg::max_corner, 0>(queryBox, ptX + parSpace  );
      bg::set<bg::max_corner, 1>(queryBox, ptY + parWithin );
    } else if (edge->getDir() == frDirEnum::W) {
      bg::set<bg::min_corner, 0>(queryBox, ptX - parSpace  );
      bg::set<bg::min_corner, 1>(queryBox, ptY - parWithin );
      bg::set<bg::max_corner, 0>(queryBox, ptX             );
      bg::set<bg::max_corner, 1>(queryBox, ptY + eolWithin );
    } else if (edge->getDir() == frDirEnum::N) {
      bg::set<bg::min_corner, 0>(queryBox, ptX - parWithin );
      bg::set<bg::min_corner, 1>(queryBox, ptY             );
      bg::set<bg::max_corner, 0>(queryBox, ptX + eolWithin );
      bg::set<bg::max_corner, 1>(queryBox, ptY + parSpace  );
    } else { // S
      bg::set<bg::min_corner, 0>(queryBox, ptX - eolWithin );
      bg::set<bg::min_corner, 1>(queryBox, ptY - parSpace  );
      bg::set<bg::max_corner, 0>(queryBox, ptX + parWithin );
      bg::set<bg::max_corner, 1>(queryBox, ptY             );
    }
  }
  gtl::xl(queryRect, queryBox.min_corner().x());
  gtl::yl(queryRect, queryBox.min_corner().y());
  gtl::xh(queryRect, queryBox.max_corner().x());
  gtl::yh(queryRect, queryBox.max_corner().y());

  //bg::set<bg::min_corner, 0>(queryBox, queryBox.min_corner().x() + 1);
  //bg::set<bg::min_corner, 1>(queryBox, queryBox.min_corner().y() + 1);
  //bg::set<bg::max_corner, 0>(queryBox, queryBox.max_corner().x() - 1);
  //bg::set<bg::max_corner, 1>(queryBox, queryBox.max_corner().y() - 1);
}

void FlexGCWorker::checkMetalEndOfLine_eol_hasParallelEdge_oneDir_getParallelEdgeRect(gcSegment *edge, gtl::rectangle_data<frCoord> &rect) {
  if (edge->getDir() == frDirEnum::E) {
    gtl::xl(rect, edge->low().x());
    gtl::yl(rect, edge->low().y());
    gtl::xh(rect, edge->high().x());
    gtl::yh(rect, edge->high().y() + 1);
  } else if (edge->getDir() == frDirEnum::W) {
    gtl::xl(rect, edge->high().x());
    gtl::yl(rect, edge->high().y() - 1);
    gtl::xh(rect, edge->low().x());
    gtl::yh(rect, edge->low().y());
  } else if (edge->getDir() == frDirEnum::N) {
    gtl::xl(rect, edge->low().x() - 1);
    gtl::yl(rect, edge->low().y());
    gtl::xh(rect, edge->high().x());
    gtl::yh(rect, edge->high().y());
  } else { // S
    gtl::xl(rect, edge->high().x());
    gtl::yl(rect, edge->high().y());
    gtl::xh(rect, edge->low().x() + 1);
    gtl::yh(rect, edge->low().y());
  }
}


bool FlexGCWorker::checkMetalEndOfLine_eol_hasParallelEdge_oneDir(gcSegment *edge, frSpacingEndOfLineConstraint *con, bool isSegLow, bool &hasRoute) {
  bool sol = false;
  auto layerNum = edge->getLayerNum();
  box_t queryBox; // (shrink by one, disabled)
  gtl::rectangle_data<frCoord> queryRect; // original size
  checkMetalEndOfLine_eol_hasParallelEdge_oneDir_getQueryBox(edge, con, isSegLow, queryBox, queryRect);
  gtl::rectangle_data<frCoord> triggerRect;

  vector<pair<segment_t, gcSegment*> > results;
  auto &workerRegionQuery = getWorkerRegionQuery();
  workerRegionQuery.queryPolygonEdge(queryBox, edge->getLayerNum(), results);
  gtl::polygon_90_set_data<frCoord> tmpPoly;
  for (auto &[boostSeg, ptr]: results) {
    // skip if non oppo-dir parallel edge
    if (isSegLow) {
      if (gtl::orientation(*ptr, *edge) != -1) {
        continue;
      }
    } else {
      if (gtl::orientation(*edge, *ptr) != -1) {
        continue;
      }
    }
    // (skip if no area, done by reducing queryBox... skipped here, --> not skipped)
    checkMetalEndOfLine_eol_hasParallelEdge_oneDir_getParallelEdgeRect(ptr, triggerRect);
    if (!gtl::intersects(queryRect, triggerRect, false)) {
      continue;
    }
    // check whether parallel edge is route or not
    sol = true;
    if (!hasRoute && !ptr->isFixed()) {
      tmpPoly.clear();
      //checkMetalEndOfLine_eol_hasParallelEdge_oneDir_getParallelEdgeRect(ptr, triggerRect);
      // tmpPoly is the intersection of queryRect and minimum paralleledge rect
      using namespace boost::polygon::operators;
      tmpPoly += queryRect;
      tmpPoly &= triggerRect;
      auto &polys = ptr->getNet()->getPolygons(layerNum, false);
      // tmpPoly should have route shapes in order to be considered route
      tmpPoly &= polys;
      if (!gtl::empty(tmpPoly)) {
        hasRoute = true;
        break;
      }
    }
  }
  return sol;
}

bool FlexGCWorker::checkMetalEndOfLine_eol_hasParallelEdge(gcSegment *edge, frSpacingEndOfLineConstraint *con, bool &hasRoute) {
  if (!con->hasParallelEdge()) {
    return true;
  }
  bool left  = checkMetalEndOfLine_eol_hasParallelEdge_oneDir(edge, con, true, hasRoute);
  bool right = checkMetalEndOfLine_eol_hasParallelEdge_oneDir(edge, con, false, hasRoute);
  if ((!con->hasTwoEdges()) && (left || right)) {
    return true;
  }
  if (con->hasTwoEdges() && left && right) {
    return true;
  }
  return false;
}

void FlexGCWorker::checkMetalEndOfLine_eol_hasEol_getQueryBox(gcSegment *edge, frSpacingEndOfLineConstraint *con, 
                                                              box_t &queryBox, gtl::rectangle_data<frCoord> &queryRect) {
  auto eolWithin = con->getEolWithin();
  auto eolSpace  = con->getMinSpacing();
  if (edge->getDir() == frDirEnum::E) {
    bg::set<bg::min_corner, 0>(queryBox, edge->low().x()  - eolWithin);
    bg::set<bg::min_corner, 1>(queryBox, edge->low().y()  - eolSpace);
    bg::set<bg::max_corner, 0>(queryBox, edge->high().x() + eolWithin);
    bg::set<bg::max_corner, 1>(queryBox, edge->high().y());
  } else if (edge->getDir() == frDirEnum::W) {
    bg::set<bg::min_corner, 0>(queryBox, edge->high().x() - eolWithin);
    bg::set<bg::min_corner, 1>(queryBox, edge->high().y());
    bg::set<bg::max_corner, 0>(queryBox, edge->low().x()  + eolWithin);
    bg::set<bg::max_corner, 1>(queryBox, edge->low().y()  + eolSpace);
  } else if (edge->getDir() == frDirEnum::N) {
    bg::set<bg::min_corner, 0>(queryBox, edge->low().x());
    bg::set<bg::min_corner, 1>(queryBox, edge->low().y()  - eolWithin);
    bg::set<bg::max_corner, 0>(queryBox, edge->high().x() + eolSpace);
    bg::set<bg::max_corner, 1>(queryBox, edge->high().y() + eolWithin);
  } else { // S
    bg::set<bg::min_corner, 0>(queryBox, edge->high().x() - eolSpace);
    bg::set<bg::min_corner, 1>(queryBox, edge->high().y() - eolWithin);
    bg::set<bg::max_corner, 0>(queryBox, edge->low().x());
    bg::set<bg::max_corner, 1>(queryBox, edge->low().y()  + eolWithin);
  }
  gtl::xl(queryRect, queryBox.min_corner().x());
  gtl::yl(queryRect, queryBox.min_corner().y());
  gtl::xh(queryRect, queryBox.max_corner().x());
  gtl::yh(queryRect, queryBox.max_corner().y());
}

void FlexGCWorker::checkMetalEndOfLine_eol_hasEol_helper(gcSegment *edge1, gcSegment *edge2, frSpacingEndOfLineConstraint *con) {
  bool enableOutput = printMarker;
  auto layerNum = edge1->getLayerNum();
  auto net1 = edge1->getNet();
  auto net2 = edge2->getNet();
  gtl::rectangle_data<frCoord> markerRect, rect2;
  checkMetalEndOfLine_eol_hasParallelEdge_oneDir_getParallelEdgeRect(edge1, markerRect);
  checkMetalEndOfLine_eol_hasParallelEdge_oneDir_getParallelEdgeRect(edge2, rect2);
  gtl::generalized_intersect(markerRect, rect2);
  // skip if markerRect contains anything
  auto &workerRegionQuery = getWorkerRegionQuery();
  vector<rq_rptr_value_t<gcRect> > result;
  gtl::rectangle_data<frCoord> bloatMarkerRect(markerRect);
  if (gtl::area(markerRect) == 0) {
    if (edge1->getDir() == frDirEnum::W || edge1->getDir() == frDirEnum::E) {
      gtl::bloat(bloatMarkerRect, gtl::HORIZONTAL, 1);
    } else if (edge1->getDir() == frDirEnum::S || edge1->getDir() == frDirEnum::N) {
      gtl::bloat(bloatMarkerRect, gtl::VERTICAL, 1);
    }
  }
  box_t queryBox(point_t(gtl::xl(bloatMarkerRect), gtl::yl(bloatMarkerRect)), 
                 point_t(gtl::xh(bloatMarkerRect), gtl::yh(bloatMarkerRect)));
  workerRegionQuery.queryMaxRectangle(queryBox, layerNum, result);
  for (auto &[objBox, objPtr]: result) {
    if (gtl::intersects(bloatMarkerRect, *objPtr, false)) {
      return;
    }
  }
  //{
  //  using namespace boost::polygon::operators;
  //  gtl::polygon_90_set_data<frCoord> polys1, polys2;
  //  polys1 += net1->getPolygons(layerNum, false);
  //  polys1 += net1->getPolygons(layerNum, true);
  //  auto intersection_polys1 = polys1 & markerRect;
  //  polys2 += net2->getPolygons(layerNum, false);
  //  polys2 += net2->getPolygons(layerNum, true);
  //  auto intersection_polys2 = polys2 & markerRect;
  //  if (!(gtl::empty(intersection_polys1) && gtl::empty(intersection_polys2))) {
  //    return;
  //  }
  //}

  auto marker = make_unique<frMarker>();
  frBox box(gtl::xl(markerRect), gtl::yl(markerRect), gtl::xh(markerRect), gtl::yh(markerRect));
  marker->setBBox(box);
  marker->setLayerNum(layerNum);
  marker->setConstraint(con);
  marker->addSrc(net1->getOwner());
  marker->addSrc(net2->getOwner());
  if (addMarker(marker)) {
    // true marker
    if (enableOutput) {
      double dbu = getDesign()->getTopBlock()->getDBUPerUU();
      cout <<"EOLSpc@(" <<gtl::xl(markerRect) / dbu <<", " <<gtl::yl(markerRect) / dbu <<") ("
                        <<gtl::xh(markerRect) / dbu <<", " <<gtl::yh(markerRect) / dbu <<") "
           <<getDesign()->getTech()->getLayer(layerNum)->getName() <<" ";
      auto owner = net1->getOwner();
      if (owner == nullptr) {
        cout <<"FLOATING";
      } else {
        if (owner->typeId() == frcNet) {
          cout <<static_cast<frNet*>(owner)->getName();
        } else if (owner->typeId() == frcInstTerm) {
          cout <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
               <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
        } else if (owner->typeId() == frcTerm) {
          cout <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
        } else if (owner->typeId() == frcInstBlockage) {
          cout <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
        } else if (owner->typeId() == frcBlockage) {
          cout <<"PIN/OBS";
        } else {
          cout <<"UNKNOWN";
        }
      }
      cout <<" ";
      owner = net2->getOwner();
      if (owner == nullptr) {
        cout <<"FLOATING";
      } else {
        if (owner->typeId() == frcNet) {
          cout <<static_cast<frNet*>(owner)->getName();
        } else if (owner->typeId() == frcInstTerm) {
          cout <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
               <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
        } else if (owner->typeId() == frcTerm) {
          cout <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
        } else if (owner->typeId() == frcInstBlockage) {
          cout <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
        } else if (owner->typeId() == frcBlockage) {
          cout <<"PIN/OBS";
        } else {
          cout <<"UNKNOWN";
        }
      }
      cout <<endl;
    }
  }

}

void FlexGCWorker::checkMetalEndOfLine_eol_hasEol(gcSegment *edge, frSpacingEndOfLineConstraint *con, bool hasRoute) {
  auto layerNum = edge->getLayerNum();
  box_t queryBox;
  gtl::rectangle_data<frCoord> queryRect; // original size
  checkMetalEndOfLine_eol_hasEol_getQueryBox(edge, con, queryBox, queryRect);

  gtl::rectangle_data<frCoord> triggerRect;
  vector<pair<segment_t, gcSegment*> > results;
  auto &workerRegionQuery = getWorkerRegionQuery();
  workerRegionQuery.queryPolygonEdge(queryBox, edge->getLayerNum(), results);
  gtl::polygon_90_set_data<frCoord> tmpPoly;
  for (auto &[boostSeg, ptr]: results) {
    // skip if non oppo-dir edge
    if ((int)edge->getDir() + (int)ptr->getDir() != OPPOSITEDIR) {
      continue;
    }
    checkMetalEndOfLine_eol_hasParallelEdge_oneDir_getParallelEdgeRect(ptr, triggerRect);
    // skip if no area
    if (!gtl::intersects(queryRect, triggerRect, false)) {
      continue;
    }
    // skip if no route shapes
    if (!hasRoute && !ptr->isFixed()) {
      tmpPoly.clear();
      // tmpPoly is the intersection of queryRect and minimum paralleledge rect
      using namespace boost::polygon::operators;
      tmpPoly += queryRect;
      tmpPoly &= triggerRect;
      auto &polys = ptr->getNet()->getPolygons(layerNum, false);
      // tmpPoly should have route shapes in order to be considered route
      tmpPoly &= polys;
      if (!gtl::empty(tmpPoly)) {
        hasRoute = true;
      }
    }
    // skip if no route
    if (!hasRoute) {
      continue;
    }
    checkMetalEndOfLine_eol_hasEol_helper(edge, ptr, con);
  }
}

void FlexGCWorker::checkMetalEndOfLine_eol(gcSegment *edge, frSpacingEndOfLineConstraint *con) {
  if (!checkMetalEndOfLine_eol_isEolEdge(edge, con)) {
    return;
  }
  auto layerNum = edge->getLayerNum();
  // check left/right parallel edge
  //auto hasRoute = edge->isFixed() ? false : true;
  // check if current eol edge has route shapes
  bool hasRoute = false;
  if (!edge->isFixed()) {
    gtl::polygon_90_set_data<frCoord> tmpPoly;
    gtl::rectangle_data<frCoord> triggerRect;
    checkMetalEndOfLine_eol_hasParallelEdge_oneDir_getParallelEdgeRect(edge, triggerRect);
    // tmpPoly is the intersection of queryRect and minimum paralleledge rect
    using namespace boost::polygon::operators;
    tmpPoly += triggerRect;
    auto &polys = edge->getNet()->getPolygons(layerNum, false);
    // tmpPoly should have route shapes in order to be considered route
    tmpPoly &= polys;
    if (!gtl::empty(tmpPoly)) {
      hasRoute = true;
    }
  }
  auto triggered = checkMetalEndOfLine_eol_hasParallelEdge(edge, con, hasRoute);
  if (!triggered) {
    return;
  }
  // check eol
  checkMetalEndOfLine_eol_hasEol(edge, con, hasRoute);
}

void FlexGCWorker::checkMetalEndOfLine_main(gcPin* pin) {
  //bool enableOutput = true;

  auto poly = pin->getPolygon();
  auto layerNum = poly->getLayerNum();
  //auto net = poly->getNet();

  auto &cons = design->getTech()->getLayer(layerNum)->getEolSpacing();
  if (cons.empty()) {
    return;
  }

  for (auto &edges: pin->getPolygonEdges()) {
    for (auto &edge: edges) {
      for (auto con: cons) {
        checkMetalEndOfLine_eol(edge.get(), con);
      }
    }
  }
}

void FlexGCWorker::checkMetalEndOfLine() {
  if (targetNet) {
    // layer --> net --> polygon
    for (int i = std::max((frLayerNum)(getDesign()->getTech()->getBottomLayerNum()), minLayerNum); 
         i <= std::min((frLayerNum)(getDesign()->getTech()->getTopLayerNum()), maxLayerNum); i++) {
      auto currLayer = getDesign()->getTech()->getLayer(i);
      if (currLayer->getType() != frLayerTypeEnum::ROUTING) {
        continue;
      }
      for (auto &pin: targetNet->getPins(i)) {
        checkMetalEndOfLine_main(pin.get());
      }
    }
  } else {
    // layer --> net --> polygon
    for (int i = std::max((frLayerNum)(getDesign()->getTech()->getBottomLayerNum()), minLayerNum); 
         i <= std::min((frLayerNum)(getDesign()->getTech()->getTopLayerNum()), maxLayerNum); i++) {
      auto currLayer = getDesign()->getTech()->getLayer(i);
      if (currLayer->getType() != frLayerTypeEnum::ROUTING) {
        continue;
      }
      for (auto &net: getNets()) {
        for (auto &pin: net->getPins(i)) {
          checkMetalEndOfLine_main(pin.get());
        }
      }
    }
  }
}

frCoord FlexGCWorker::checkCutSpacing_getMaxSpcVal(frCutSpacingConstraint* con) {
  frCoord maxSpcVal = 0;
  if (con) {
    maxSpcVal = con->getCutSpacing();
    if (con->isAdjacentCuts()) {
      maxSpcVal = std::max(maxSpcVal, con->getCutWithin());
    }
  }
  return maxSpcVal;
}

frCoord FlexGCWorker::checkCutSpacing_spc_getReqSpcVal(gcRect* ptr1, gcRect* ptr2, frCutSpacingConstraint* con) {
  frCoord maxSpcVal = 0;
  if (con) {
    maxSpcVal = con->getCutSpacing();
    if (con->isAdjacentCuts()) {
      auto owner = ptr1->getNet()->getOwner();
      if (owner && (owner->typeId() == frcInstBlockage || owner->typeId() == frcBlockage)) {
        maxSpcVal = std::max(maxSpcVal, con->getCutWithin());
      }
      owner = ptr2->getNet()->getOwner();
      if (owner && (owner->typeId() == frcInstBlockage || owner->typeId() == frcBlockage)) {
        maxSpcVal = std::max(maxSpcVal, con->getCutWithin());
      }
    }
  }
  return maxSpcVal;
}

void FlexGCWorker::checkCutSpacing_short(gcRect* rect1, gcRect* rect2, const gtl::rectangle_data<frCoord> &markerRect) {
  bool enableOutput = printMarker;

  auto layerNum = rect1->getLayerNum();
  auto net1 = rect1->getNet();
  auto net2 = rect2->getNet();
  // skip fixed shape
  if (rect1->isFixed() && rect2->isFixed()) {
    return;
  }

  auto marker = make_unique<frMarker>();
  frBox box(gtl::xl(markerRect), gtl::yl(markerRect), gtl::xh(markerRect), gtl::yh(markerRect));
  marker->setBBox(box);
  marker->setLayerNum(layerNum);
  marker->setConstraint(getDesign()->getTech()->getLayer(layerNum)->getShortConstraint());
  marker->addSrc(net1->getOwner());
  marker->addSrc(net2->getOwner());
  if (addMarker(marker)) {
    // true marker
    if (enableOutput) {
      double dbu = getDesign()->getTopBlock()->getDBUPerUU();
      cout <<"CShort@(";
      cout <<gtl::xl(markerRect) / dbu <<", " <<gtl::yl(markerRect) / dbu <<") ("
           <<gtl::xh(markerRect) / dbu <<", " <<gtl::yh(markerRect) / dbu <<") "
           <<getDesign()->getTech()->getLayer(layerNum)->getName() <<" ";
      auto owner = net1->getOwner();
      if (owner == nullptr) {
        cout <<"FLOATING";
      } else {
        if (owner->typeId() == frcNet) {
          cout <<static_cast<frNet*>(owner)->getName();
        } else if (owner->typeId() == frcInstTerm) {
          cout <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
               <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
        } else if (owner->typeId() == frcTerm) {
          cout <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
        } else if (owner->typeId() == frcInstBlockage) {
          cout <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
        } else if (owner->typeId() == frcBlockage) {
          cout <<"PIN/OBS";
        } else {
          cout <<"UNKNOWN";
        }
      }
      cout <<" ";
      owner = net2->getOwner();
      if (owner == nullptr) {
        cout <<"FLOATING";
      } else {
        if (owner->typeId() == frcNet) {
          cout <<static_cast<frNet*>(owner)->getName();
        } else if (owner->typeId() == frcInstTerm) {
          cout <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
               <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
        } else if (owner->typeId() == frcTerm) {
          cout <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
        } else if (owner->typeId() == frcInstBlockage) {
          cout <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
        } else if (owner->typeId() == frcBlockage) {
          cout <<"PIN/OBS";
        } else {
          cout <<"UNKNOWN";
        }
      }
      cout <<endl;
    }
  }
}

void FlexGCWorker::checkCutSpacing_spc(gcRect* rect1, gcRect* rect2, const gtl::rectangle_data<frCoord> &markerRect, 
                                       frCutSpacingConstraint* con, frCoord prl) {
  bool enableOutput = printMarker;

  auto layerNum = rect1->getLayerNum();
  auto net1 = rect1->getNet();
  auto net2 = rect2->getNet();
  // skip if adjcut && except samepgnet, note that adj cut handled given only rect1 outside this function
  if (con->isAdjacentCuts() && con->hasExceptSamePGNet() && net1 == net2 && net1->getOwner()) {
    auto owner = net1->getOwner();
    if (owner->typeId() == frcNet) {
      if (static_cast<frNet*>(owner)->getType() == frNetEnum::frcPowerNet ||
          static_cast<frNet*>(owner)->getType() == frNetEnum::frcGroundNet) {
        return;
      }
    } else if (owner->typeId() == frcTerm) {
      if (static_cast<frTerm*>(owner)->getType() == frTermEnum::frcPowerTerm ||
          static_cast<frTerm*>(owner)->getType() == frTermEnum::frcGroundTerm) {
        return;
      }
    } else if (owner->typeId() == frcInstTerm) {
      if (static_cast<frInstTerm*>(owner)->getTerm()->getType() == frTermEnum::frcPowerTerm ||
          static_cast<frInstTerm*>(owner)->getTerm()->getType() == frTermEnum::frcGroundTerm) {
        return;
      }
    }
  }
  if (con->isParallelOverlap()) {
    // skip if no parallel overlap
    if (prl <= 0) {
      return;
    // skip if paralell overlap but shares the same above/below metal
    } else {
      box_t queryBox;
      myBloat(markerRect, 0, queryBox);
      vector<rq_rptr_value_t<gcRect> > results;
      auto &workerRegionQuery = getWorkerRegionQuery();
      vector<rq_rptr_value_t<gcRect> > result;
      auto secondLayerNum = rect1->getLayerNum() - 1;
      if (secondLayerNum >= getDesign()->getTech()->getBottomLayerNum() &&
          secondLayerNum <= getDesign()->getTech()->getTopLayerNum()) {
        workerRegionQuery.queryMaxRectangle(queryBox, secondLayerNum, result);
      }
      secondLayerNum = rect1->getLayerNum() + 1;
      if (secondLayerNum >= getDesign()->getTech()->getBottomLayerNum() &&
          secondLayerNum <= getDesign()->getTech()->getTopLayerNum()) {
        workerRegionQuery.queryMaxRectangle(queryBox, secondLayerNum, result);
      }
      for (auto &[objBox, objPtr]: result) {
        if ((objPtr->getNet() == net1 || objPtr->getNet() == net2) && bg::covered_by(queryBox, objBox)) {
          return;
        }
      }
    }
  }
  // skip if not reaching area
  if (con->isArea() && gtl::area(*rect1) < con->getCutArea() && gtl::area(*rect2) < con->getCutArea()) {
    return;
  }

  // no violation if spacing satisfied
  auto reqSpcValSquare = checkCutSpacing_spc_getReqSpcVal(rect1, rect2, con);
  reqSpcValSquare *= reqSpcValSquare;

  gtl::point_data<frCoord> center1, center2;
  gtl::center(center1, *rect1);
  gtl::center(center2, *rect2);
  frCoord distSquare = 0;
  if (con->hasCenterToCenter()) {
    distSquare = gtl::distance_squared(center1, center2);
  } else {
    distSquare = gtl::square_euclidean_distance(*rect1, *rect2);
  }
  if (distSquare >= reqSpcValSquare) {
    return;
  }
  // no violation if fixed shapes
  if (rect1->isFixed() && rect2->isFixed()) {
    return;
  }

  auto marker = make_unique<frMarker>();
  frBox box(gtl::xl(markerRect), gtl::yl(markerRect), gtl::xh(markerRect), gtl::yh(markerRect));
  marker->setBBox(box);
  marker->setLayerNum(layerNum);
  marker->setConstraint(con);
  marker->addSrc(net1->getOwner());
  marker->addSrc(net2->getOwner());
  if (addMarker(marker)) {
    // true marker
    if (enableOutput) {
      double dbu = getDesign()->getTopBlock()->getDBUPerUU();
      cout <<"CutSpc@(" <<gtl::xl(markerRect) / dbu <<", " <<gtl::yl(markerRect) / dbu <<") ("
                        <<gtl::xh(markerRect) / dbu <<", " <<gtl::yh(markerRect) / dbu <<") "
           <<getDesign()->getTech()->getLayer(layerNum)->getName() <<" ";
      auto owner = net1->getOwner();
      if (owner == nullptr) {
        cout <<"FLOATING";
      } else {
        if (owner->typeId() == frcNet) {
          cout <<static_cast<frNet*>(owner)->getName();
        } else if (owner->typeId() == frcInstTerm) {
          cout <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
               <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
        } else if (owner->typeId() == frcTerm) {
          cout <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
        } else if (owner->typeId() == frcInstBlockage) {
          cout <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
        } else if (owner->typeId() == frcBlockage) {
          cout <<"PIN/OBS";
        } else {
          cout <<"UNKNOWN";
        }
      }
      cout <<" ";
      owner = net2->getOwner();
      if (owner == nullptr) {
        cout <<"FLOATING";
      } else {
        if (owner->typeId() == frcNet) {
          cout <<static_cast<frNet*>(owner)->getName();
        } else if (owner->typeId() == frcInstTerm) {
          cout <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
               <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
        } else if (owner->typeId() == frcTerm) {
          cout <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
        } else if (owner->typeId() == frcInstBlockage) {
          cout <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
        } else if (owner->typeId() == frcBlockage) {
          cout <<"PIN/OBS";
        } else {
          cout <<"UNKNOWN";
        }
      }
      cout <<endl;
    }
  }
}

// check short for every spacing rule except layer
void FlexGCWorker::checkCutSpacing_main(gcRect* ptr1, gcRect* ptr2, frCutSpacingConstraint* con) {
  // skip if same obj
  if (ptr1 == ptr2) {
    return;
  }
  // skip if con is not same net rule, but layer has same net rule and are same net
  if (!getDesign()->getTech()->getLayer(ptr1->getLayerNum())->getCutSpacing(true).empty() &&
      !con->hasSameNet() &&
      ptr1->getNet() == ptr2->getNet()) {
    return;
  }
  gtl::rectangle_data<frCoord> markerRect(*ptr1);
  auto distX = gtl::euclidean_distance(markerRect, *ptr2, gtl::HORIZONTAL);
  auto distY = gtl::euclidean_distance(markerRect, *ptr2, gtl::VERTICAL);

  gtl::generalized_intersect(markerRect, *ptr2);
  auto prlX = gtl::delta(markerRect, gtl::HORIZONTAL);
  auto prlY = gtl::delta(markerRect, gtl::VERTICAL);

  if (distX) {
    prlX = -prlX;
  }
  if (distY) {
    prlY = -prlY;
  }
  
  // CShort
  if (distX == 0 && distY == 0) {
    checkCutSpacing_short(ptr1, ptr2, markerRect);
  // CutSpc
  } else {
    checkCutSpacing_spc(ptr1, ptr2, markerRect, con, std::max(prlX, prlY));
  }
}

bool FlexGCWorker::checkCutSpacing_main_hasAdjCuts(gcRect* rect, frCutSpacingConstraint* con) {
  // no adj cut rule, must proceed checking
  if (!con->isAdjacentCuts()) {
    return true;
  }
  // rect is obs, must proceed regardless of numCuts
  if (rect->getNet()->getOwner() && 
      (rect->getNet()->getOwner()->typeId() == frcInstBlockage ||
       rect->getNet()->getOwner()->typeId() == frcBlockage)) {
    return true;
  }

  auto layerNum = rect->getLayerNum();
  auto cutWithinSquare = con->getCutWithin();
  box_t queryBox;
  myBloat(*rect, cutWithinSquare, queryBox);
  cutWithinSquare *= cutWithinSquare;
  auto &workerRegionQuery = getWorkerRegionQuery();
  vector<rq_rptr_value_t<gcRect> > result;
  workerRegionQuery.queryMaxRectangle(queryBox, layerNum, result);
  int reqNumCut = con->getAdjacentCuts();
  int cnt = -1;
  gtl::point_data<frCoord> center1, center2;
  gtl::center(center1, *rect);
  // count adj cuts
  for (auto &[objBox, ptr]: result) {
    frCoord distSquare = 0;
    if (con->hasCenterToCenter()) {
      gtl::center(center2, *ptr);
      distSquare = gtl::distance_squared(center1, center2);
    } else {
      distSquare = gtl::square_euclidean_distance(*rect, *ptr);
    }
    if (distSquare >= cutWithinSquare) {
      continue;
    }
    if (ptr->getNet()->getOwner() && 
        (ptr->getNet()->getOwner()->typeId() == frcInstBlockage ||
         ptr->getNet()->getOwner()->typeId() == frcBlockage)) {
      cnt += reqNumCut;
    } else {
      cnt++;
    }
  }
  if (cnt >= reqNumCut) {
    return true;
  } else {
    return false;
  }
}

void FlexGCWorker::checkCutSpacing_main(gcRect* rect, frCutSpacingConstraint* con) {
  //bool enableOutput = true;
  bool enableOutput = false;
  if (con && con->isLayer()) {
    cout <<"Error: secondLayerName not supported" <<endl;
    exit(1);
  }
  auto layerNum = rect->getLayerNum();
  auto maxSpcVal = checkCutSpacing_getMaxSpcVal(con);
  box_t queryBox;
  myBloat(*rect, maxSpcVal, queryBox);
  if (enableOutput) {
    double dbu = getDesign()->getTopBlock()->getDBUPerUU();
    cout <<"checkCutSpacing maxRect ";
    if (rect->isFixed()) {
      cout <<"FIXED";
    } else {
      cout <<"ROUTE";
    }
    cout <<" (" <<gtl::xl(*rect) / dbu <<", " <<gtl::yl(*rect) / dbu <<") (" 
                <<gtl::xh(*rect) / dbu <<", " <<gtl::yh(*rect) / dbu <<") "
         <<getDesign()->getTech()->getLayer(layerNum)->getName() <<" ";
    cout <<"bloat maxSpcVal@" <<maxSpcVal / dbu <<" (" <<queryBox.min_corner().x() / dbu <<", " <<queryBox.min_corner().x() / dbu <<") (" 
                      <<queryBox.max_corner().x() / dbu <<", " <<queryBox.max_corner().x() / dbu <<") ";
    auto owner = rect->getNet()->getOwner();
    if (owner == nullptr) {
      cout <<" FLOATING";
    } else {
      if (owner->typeId() == frcNet) {
        cout <<static_cast<frNet*>(owner)->getName();
      } else if (owner->typeId() == frcInstTerm) {
        cout <<static_cast<frInstTerm*>(owner)->getInst()->getName() <<"/" 
             <<static_cast<frInstTerm*>(owner)->getTerm()->getName();
      } else if (owner->typeId() == frcTerm) {
        cout <<"PIN/" <<static_cast<frTerm*>(owner)->getName();
      } else if (owner->typeId() == frcInstBlockage) {
        cout <<static_cast<frInstBlockage*>(owner)->getInst()->getName() <<"/OBS";
      } else if (owner->typeId() == frcBlockage) {
        cout <<"PIN/OBS";
      } else {
        cout <<"UNKNOWN";
      }
    }
    cout <<endl;
  }

  // skip if adjcut not satisfied
  if (!checkCutSpacing_main_hasAdjCuts(rect, con)) {
    return;
  }

  auto &workerRegionQuery = getWorkerRegionQuery();
  vector<rq_rptr_value_t<gcRect> > result;
  workerRegionQuery.queryMaxRectangle(queryBox, layerNum, result);
  // Short, metSpc, NSMetal here
  for (auto &[objBox, ptr]: result) {
    //cout <<"from " <<rect <<" to " <<ptr <<endl;
    checkCutSpacing_main(rect, ptr, con);
  }
}

void FlexGCWorker::checkCutSpacing_main(gcRect* rect) {
  auto layerNum = rect->getLayerNum();

  // CShort
  //checkCutSpacing_main(rect, nullptr);
  // diff net
  for (auto con: getDesign()->getTech()->getLayer(layerNum)->getCutSpacing(false)) {
    checkCutSpacing_main(rect, con);
  }
  // same net
  for (auto con: getDesign()->getTech()->getLayer(layerNum)->getCutSpacing(true)) {
    checkCutSpacing_main(rect, con);
  }
}

void FlexGCWorker::checkCutSpacing() {
  if (targetNet) {
    // layer --> net --> polygon --> maxrect
    for (int i = std::max((frLayerNum)(getDesign()->getTech()->getBottomLayerNum()), minLayerNum); 
         i <= std::min((frLayerNum)(getDesign()->getTech()->getTopLayerNum()), maxLayerNum); i++) {
      auto currLayer = getDesign()->getTech()->getLayer(i);
      if (currLayer->getType() != frLayerTypeEnum::CUT) {
        continue;
      }
      for (auto &pin: targetNet->getPins(i)) {
        for (auto &maxrect: pin->getMaxRectangles()) {
          checkCutSpacing_main(maxrect.get());
        }
      }
    }
  } else {
    // layer --> net --> polygon --> maxrect
    for (int i = std::max((frLayerNum)(getDesign()->getTech()->getBottomLayerNum()), minLayerNum); 
         i <= std::min((frLayerNum)(getDesign()->getTech()->getTopLayerNum()), maxLayerNum); i++) {
      auto currLayer = getDesign()->getTech()->getLayer(i);
      if (currLayer->getType() != frLayerTypeEnum::CUT) {
        continue;
      }
      for (auto &net: getNets()) {
        for (auto &pin: net->getPins(i)) {
          for (auto &maxrect: pin->getMaxRectangles()) {
            //cout <<"from " <<maxrect.get() <<endl;
            checkCutSpacing_main(maxrect.get());
          }
        }
      }
    }
  }
}



int FlexGCWorker::main() {
  //printMarker = true;
  // incremental updates
  if (!modifiedDRNets.empty()) {
    updateGCWorker();
  }
  // clear existing markers
  clearMarkers();
  // check Short, NSMet, MetSpc based on max rectangles
  checkMetalSpacing();
  // check MinWid, MinStp based on polygon
  checkMetalShape();
  // check eolSpc based on polygon
  checkMetalEndOfLine();
  // check CShort, cutSpc
  checkCutSpacing();
  return 0;
}
