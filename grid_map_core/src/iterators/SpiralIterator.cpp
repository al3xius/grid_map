/*
 * SpiralIterator.hpp
 *
 *  Created on: Jul 7, 2015
 *      Author: Martin Wermelinger
 *   Institute: ETH Zurich, ANYbotics
 */

#include "grid_map_core/iterators/SpiralIterator.hpp"
#include "grid_map_core/GridMapMath.hpp"

#include <cmath>
#include <utility>


namespace grid_map {

SpiralIterator::SpiralIterator(const grid_map::GridMap& gridMap, Eigen::Vector2d  center,
                               const double radius)
    : center_(std::move(center)),
      radius_(radius),
      distance_(0)
{
  radiusSquare_ = radius_ * radius_;
  mapLength_ = gridMap.getLength();
  mapPosition_ = gridMap.getPosition();
  resolution_ = gridMap.getResolution();
  bufferSize_ = gridMap.getSize();
  gridMap.getIndex(center_, indexCenter_);
  nRings_ = static_cast<uint>(std::ceil(radius_ / resolution_));
  if (checkIfIndexInRange(indexCenter_, bufferSize_)) {
    pointsRing_.push_back(indexCenter_);
  } else {
    while (pointsRing_.empty() && !isPastEnd()) {
      generateRing();
    }
  }
}

bool SpiralIterator::operator !=(const SpiralIterator& /*other*/) const
{
  return (pointsRing_.back() != pointsRing_.back()).any();
}

const Eigen::Array2i& SpiralIterator::operator *() const
{
  return pointsRing_.back();
}

SpiralIterator& SpiralIterator::operator ++()
{
  pointsRing_.pop_back();
  if (pointsRing_.empty() && !isPastEnd()) {
    generateRing();
  }
  return *this;
}

bool SpiralIterator::isPastEnd() const
{
  return (distance_ == nRings_ && pointsRing_.empty());
}

bool SpiralIterator::isInside(const Index& index) const
{
  Eigen::Vector2d position;
  getPositionFromIndex(position, index, mapLength_, mapPosition_, resolution_, bufferSize_);
  double squareNorm = (position - center_).array().square().sum();
  return (squareNorm <= radiusSquare_);
}

void SpiralIterator::generateRing()
{
  distance_++;
  Index point(distance_, 0);
  Index pointInMap;
  Index normal;
  do {
    pointInMap.x() = point.x() + indexCenter_.x();
    pointInMap.y() = point.y() + indexCenter_.y();
    if (checkIfIndexInRange(pointInMap, bufferSize_)) {
      if (distance_ == nRings_ || distance_ == nRings_ - 1) {
        if (isInside(pointInMap)) {
          pointsRing_.push_back(pointInMap);
        }
      } else {
        pointsRing_.push_back(pointInMap);
      }
    }
    normal.x() = -signum(point.y());
    normal.y() = signum(point.x());
    if (normal.x() != 0 && static_cast<unsigned int>(Vector(point.x() + normal.x(), point.y()).norm()) == distance_) {
      point.x() += normal.x();
    } else if (normal.y() != 0 && static_cast<unsigned int>(Vector(point.x(), point.y() + normal.y()).norm()) == distance_) {
      point.y() += normal.y();
    } else {
      point.x() += normal.x();
      point.y() += normal.y();
    }
  } while (static_cast<unsigned int>(point.x()) != distance_ || point.y() != 0);
}

double SpiralIterator::getCurrentRadius() const
{
  Index radius = *(*this) - indexCenter_;
  return radius.matrix().norm() * resolution_;
}

} /* namespace grid_map */

