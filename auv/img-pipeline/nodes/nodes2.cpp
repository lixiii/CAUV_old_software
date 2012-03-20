#include "../nodeFactory.h"

#include "resizeNode.h"
#include "levelsNode.h"
#include "fastCornersNode.h"
#include "drawKeyPointsNode.h"
#include "gaussianBlurNode.h"
#include "fastMedianNode.h"
#include "bearingRangeToXYNode.h"
#include "transformKeyPointsNode.h"
#include "correlation1DNode.h"
#include "rotateNode.h"
#include "renderStringNode.h"
#include "globalMaximumNode.h"
#include "localMaximaNode.h"
#include "firstAboveThresholdNode.h"

namespace cauv{
namespace imgproc{

DEFINE_NFR(ResizeNode, NodeType::Resize);
DEFINE_NFR(LevelsNode, NodeType::Levels);
DEFINE_NFR(FASTCornersNode, NodeType::FASTCorners);
DEFINE_NFR(DrawKeyPointsNode, NodeType::DrawKeyPoints);
DEFINE_NFR(GaussianBlurNode, NodeType::GaussianBlur);
DEFINE_NFR(FastMedianNode, NodeType::FastMedian);
DEFINE_NFR(BearingRangeToXYNode, NodeType::BearingRangeToXY);
DEFINE_NFR(TransformKeyPointsNode, NodeType::TransformKeyPoints);
DEFINE_NFR(Correlation1DNode, NodeType::Correlation1D);
DEFINE_NFR(RotateNode, NodeType::Rotate);
DEFINE_NFR(RenderStringNode, NodeType::RenderString);
DEFINE_NFR(GlobalMaximumNode, NodeType::GlobalMaximum);
DEFINE_NFR(LocalMaximaNode, NodeType::LocalMaxima);
DEFINE_NFR(FirstAboveThresholdNode, NodeType::FirstAboveThreshold);

} // namespace imgproc
} // namespace cauv


