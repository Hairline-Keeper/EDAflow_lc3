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

#ifndef _FR_FLEXGC_H_
#define _FR_FLEXGC_H_

#include <memory>
#include "frDesign.h"
#include "db/gcObj/gcNet.h"

namespace fr {
  class FlexGCWorker;
  class FlexGCWorkerRegionQuery {
  public:
      FlexGCWorkerRegionQuery(FlexGCWorker* in): gcWorker(in) {}
      FlexGCWorker* getGCWorker() const {
        return gcWorker;
      }
      void addPolygonEdge(gcSegment* edge);
      void addPolygonEdge(gcSegment* edge, std::vector<std::vector<std::pair<segment_t, gcSegment*> > > &allShapes);
      void addMaxRectangle(gcRect* rect);
      void addMaxRectangle(gcRect* rect, std::vector<std::vector<rq_rptr_value_t<gcRect> > > &allShapes);
      void removePolygonEdge(gcSegment* connFig);
      void removeMaxRectangle(gcRect* connFig);
      void queryPolygonEdge(const box_t &box, frLayerNum layerNum, std::vector<std::pair<segment_t, gcSegment*> > &result);
      void queryPolygonEdge(const frBox &box, frLayerNum layerNum, std::vector<std::pair<segment_t, gcSegment*> > &result);
      void queryMaxRectangle(const box_t &box, frLayerNum layerNum, std::vector<rq_rptr_value_t<gcRect> > &result);
      void queryMaxRectangle(const frBox &box, frLayerNum layerNum, std::vector<rq_rptr_value_t<gcRect> > &result);
      void queryMaxRectangle(const gtl::rectangle_data<frCoord> &box, frLayerNum layerNum, std::vector<rq_rptr_value_t<gcRect> > &result);
      void init(int numLayers);
      void addToRegionQuery(gcNet* net);
      void removeFromRegionQuery(gcNet* net);
  protected:
      FlexGCWorker* gcWorker;
      std::vector<bgi::rtree<std::pair<segment_t, gcSegment*>, bgi::quadratic<16> > > polygon_edges; // merged
      std::vector<bgi::rtree<rq_rptr_value_t<gcRect>,          bgi::quadratic<16> > > max_rectangles; // merged
  };

  class FlexDRWorker;
  class drConnFig;
  class drNet;
  class FlexGCWorker {
  public:
    // constructors
    FlexGCWorker(frDesign* designIn, FlexDRWorker* drWorkerIn = nullptr): design(designIn), drWorker(drWorkerIn), 
                 extBox(), drcBox(), owner2nets(), nets(), markers(), mapMarkers(), rq(this), printMarker(false), modifiedDRNets(),
                 modifiedOwners(), modifiedOwnersSet(),
                 targetNet(nullptr), minLayerNum(std::numeric_limits<frLayerNum>::min()), maxLayerNum(std::numeric_limits<frLayerNum>::max()),
                 targetObj(nullptr), ignoreDB(false) {}
    // setters
    void setExtBox(const frBox &in) {
      extBox.set(in);
    }
    void setDrcBox(const frBox &in) {
      drcBox.set(in);
    }
    gcNet* addNet(frBlockObject* owner = nullptr) {
      auto uNet = std::make_unique<gcNet>(design->getTech()->getLayers().size());
      auto net = uNet.get();
      net->setOwner(owner);
      nets.push_back(std::move(uNet));
      owner2nets[owner] = net;
      return net;
    }
    bool addMarker(std::unique_ptr<frMarker> &in) {
      frBox bbox;
      in->getBBox(bbox);
      auto layerNum = in->getLayerNum();
      auto con = in->getConstraint();
      std::vector<frBlockObject*> srcs(2, nullptr);
      int i = 0;
      for (auto &src: in->getSrcs()) {
        srcs.at(i) = src;
        i++;
      }
      if (mapMarkers.find(std::make_tuple(bbox, layerNum, con, srcs[0], srcs[1])) != mapMarkers.end()) {
        return false;
      }
      if (mapMarkers.find(std::make_tuple(bbox, layerNum, con, srcs[1], srcs[0])) != mapMarkers.end()) {
        return false;
      }
      mapMarkers[std::make_tuple(bbox, layerNum, con, srcs[0], srcs[1])] = in.get();
      markers.push_back(std::move(in));
      return true;
    }
    void clearMarkers() {
      mapMarkers.clear();
      markers.clear();
    }
    void setTargetObj(frBlockObject* in) {
      targetObj = in;
    }
    void setIgnoreDB() {
      ignoreDB = true;
    }
    void addPAObj(frConnFig* obj, frBlockObject* owner);
    // getters
    frDesign* getDesign() const {
      return design;
    }
    FlexDRWorker* getDRWorker() const {
      return drWorker;
    }
    void getExtBox(frBox &in) const {
      in.set(extBox);
    }
    const frBox& getExtBox() const {
      return extBox;
    }
    void getDrcBox(frBox &in) const {
      in.set(drcBox);
    }
    const frBox& getDrcBox() const {
      return drcBox;
    }
    const std::vector<std::unique_ptr<frMarker> >& getMarkers() const {
      return markers;
    }
    frRegionQuery* getRegionQuery() const {
      return design->getRegionQuery();
    }
    FlexGCWorkerRegionQuery& getWorkerRegionQuery() {
      return rq;
    }
    std::vector<std::unique_ptr<gcNet> >& getNets() {
      return nets;
    }
    // others
    void init();
    int  main();
    void end();
    int  test();
    // initialization from FlexPA, initPA0 --> addPAObj --> initPA1
    void initPA0();
    void initPA1();
    void resetPAOwner(frBlockObject* owner);
    void initPA2();
    void updateDRNet(drNet* net);

  protected:
    frDesign*                            design;
    FlexDRWorker*                        drWorker;

    frBox                                extBox;
    frBox                                drcBox;

    std::map<frBlockObject*, gcNet*>     owner2nets; // no order is assumed
    std::vector<std::unique_ptr<gcNet> > nets;

    std::vector<std::unique_ptr<frMarker> > markers;
    std::map<std::tuple<frBox, frLayerNum, frConstraint*, frBlockObject*, frBlockObject*>, frMarker*> mapMarkers;

    FlexGCWorkerRegionQuery              rq;
    bool                                 printMarker;

    // temps
    std::vector<drNet*>                  modifiedDRNets;
    std::vector<frBlockObject*>          modifiedOwners;
    std::set<frBlockObject*>             modifiedOwnersSet;

    // parameters
    gcNet*                               targetNet;
    frLayerNum                           minLayerNum;
    frLayerNum                           maxLayerNum;

    // for pin prep
    frBlockObject*                       targetObj;
    bool                                 ignoreDB;

    // init
    gcNet* getNet(frBlockObject* obj);
    void initObj(const frBox &box, frLayerNum layerNum, frBlockObject* obj, bool isFixed);
    void initDRObj(drConnFig* obj, gcNet* currNet = nullptr);
    void initDesign();
    bool initDesign_skipObj(frBlockObject* obj);
    void initDRWorker();
    void initNets();
    void initNet(gcNet* net);
    void initNet_pins_polygon(gcNet* net);
    void initNet_pins_polygonEdges(gcNet* net);
    void initNet_pins_polygonEdges_getFixedPolygonEdges(gcNet* net, std::vector<std::set<std::pair<frPoint, frPoint> > > &fixedPolygonEdges);
    void initNet_pins_polygonEdges_helper_outer(gcNet* net, gcPin* pin, gcPolygon* poly, frLayerNum i, 
                                                const std::vector<std::set<std::pair<frPoint, frPoint> > > &fixedPolygonEdges);
    void initNet_pins_polygonEdges_helper_inner(gcNet* net, gcPin* pin, const gtl::polygon_90_data<frCoord> &hole_poly, frLayerNum i, 
                                                const std::vector<std::set<std::pair<frPoint, frPoint> > > &fixedPolygonEdges);
    void initNet_pins_maxRectangles(gcNet* net);
    void initNet_pins_maxRectangles_getFixedMaxRectangles(gcNet* net, std::vector<std::set<std::pair<frPoint, frPoint> > > &fixedMaxRectangles);
    void initNet_pins_maxRectangles_helper(gcNet* net, gcPin* pin, const gtl::rectangle_data<frCoord>& rect, frLayerNum i,
                                           const std::vector<std::set<std::pair<frPoint, frPoint> > > &fixedMaxRectangles);

    void initRegionQuery();

    // update
    void updateGCWorker();

    void checkMetalSpacing();
    frCoord checkMetalSpacing_getMaxSpcVal(frLayerNum layerNum);
    void myBloat(const gtl::rectangle_data<frCoord> &rect, frCoord val, box_t &box);
    void checkMetalSpacing_main(gcRect* rect);
    void checkMetalSpacing_main(gcRect* rect1, gcRect* rect2);
    void checkMetalSpacing_short(gcRect* rect1, gcRect* rect2, const gtl::rectangle_data<frCoord> &markerRect);
    bool checkMetalSpacing_short_skipOBSPin(gcRect* rect1, gcRect* rect2, const gtl::rectangle_data<frCoord> &markerRect);
    frCoord checkMetalSpacing_prl_getReqSpcVal(gcRect* rect1, gcRect* rect2, frCoord prl);
    bool checkMetalSpacing_prl_hasPolyEdge(gcRect* rect1, gcRect* rect2, const gtl::rectangle_data<frCoord> &markerRect, int type, frCoord prl);
    void checkMetalSpacing_prl(gcRect* rect1, gcRect* rect2, const gtl::rectangle_data<frCoord> &markerRect, frCoord prl, frCoord distX, frCoord distY);

    void checkMetalShape();
    void checkMetalShape_main(gcPin* pin);
    void checkMetalShape_minWidth(const gtl::rectangle_data<frCoord> &rect, frLayerNum layerNum, gcNet* net, bool isH);
    void checkMetalShape_offGrid(gcPin* pin);
    void checkMetalShape_minEnclosedArea(gcPin* pin);
    void checkMetalShape_minStep(gcPin* pin);
    void checkMetalShape_minStep_helper(const frBox &markerBox, frLayerNum layerNum, gcNet* net, frMinStepConstraint* con,
                                        bool hasInsideCorner, bool hasOutsideCorner, bool hasStep,
                                        int currEdges, frCoord currLength, bool hasRoute);

    void checkMetalEndOfLine();
    void checkMetalEndOfLine_main(gcPin* pin);
    void checkMetalEndOfLine_eol(gcSegment* edge, frSpacingEndOfLineConstraint* con);
    bool checkMetalEndOfLine_eol_isEolEdge(gcSegment *edge, frSpacingEndOfLineConstraint *con);
    bool checkMetalEndOfLine_eol_hasParallelEdge(gcSegment *edge, frSpacingEndOfLineConstraint *con, bool &hasRoute);
    bool checkMetalEndOfLine_eol_hasParallelEdge_oneDir(gcSegment *edge, frSpacingEndOfLineConstraint *con, bool isSegLow, bool &hasRoute);
    void checkMetalEndOfLine_eol_hasParallelEdge_oneDir_getQueryBox(gcSegment *edge, frSpacingEndOfLineConstraint *con, 
                                                                    bool isSegLow, box_t &queryBox, gtl::rectangle_data<frCoord> &queryRect);
    void checkMetalEndOfLine_eol_hasParallelEdge_oneDir_getParallelEdgeRect(gcSegment *edge, gtl::rectangle_data<frCoord> &rect);
    void checkMetalEndOfLine_eol_hasEol(gcSegment *edge, frSpacingEndOfLineConstraint *con, bool hasRoute);
    void checkMetalEndOfLine_eol_hasEol_getQueryBox(gcSegment *edge, frSpacingEndOfLineConstraint *con, 
                                                    box_t &queryBox, gtl::rectangle_data<frCoord> &queryRect);
    void checkMetalEndOfLine_eol_hasEol_helper(gcSegment *edge1, gcSegment *edge2, frSpacingEndOfLineConstraint *con);

    void checkCutSpacing();
    void checkCutSpacing_main(gcRect* rect);
    void checkCutSpacing_main(gcRect* rect, frCutSpacingConstraint* con);
    bool checkCutSpacing_main_hasAdjCuts(gcRect* rect, frCutSpacingConstraint* con);
    frCoord checkCutSpacing_getMaxSpcVal(frCutSpacingConstraint* con);
    void checkCutSpacing_main(gcRect* ptr1, gcRect* ptr2, frCutSpacingConstraint* con);
    void checkCutSpacing_short(gcRect* rect1, gcRect* rect2, const gtl::rectangle_data<frCoord> &markerRect);
    void checkCutSpacing_spc(gcRect* rect1, gcRect* rect2, const gtl::rectangle_data<frCoord> &markerRect, frCutSpacingConstraint* con, frCoord prl);
    frCoord checkCutSpacing_spc_getReqSpcVal(gcRect* ptr1, gcRect* ptr2, frCutSpacingConstraint* con);

  };
}

#endif
