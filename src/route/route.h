/*****************************************************************************
* Copyright 2015-2017 Alexander Barthel albar965@mailbox.org
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#ifndef LITTLENAVMAP_ROUTE_H
#define LITTLENAVMAP_ROUTE_H

#include "route/routeleg.h"

#include "fs/pln/flightplan.h"

class CoordinateConverter;
class FlightplanEntryBuilder;

/*
 * Aggregates the flight plan and is a list of all route map objects. Also contains and stores information
 * about the active route leg and current aircraft position.
 */
class Route :
  private QList<RouteLeg>
{
public:
  Route();
  Route(const Route& other);
  virtual ~Route();

  Route& operator=(const Route& other);

  /* Update positions, distances and try to select next leg*/
  void updateActiveLegAndPos(const map::PosCourse& pos);
  void updateActiveLegAndPos();

  /*
   * Get multiple flight plan distances for the given position. If value pointers are null they will be ignored.
   * All distances in nautical miles.
   * @param pos
   * @param distFromStartNm Distance from departure
   * @param distToDestNm Distance to destination
   * @param nextLegDistance Distance to next leg
   * @param crossTrackDistance Cross track distance to current leg. Positive and
   * negative values indicate left or right of track.
   * @param nextLegIndex Index of next leg
   * @param nextRouteLegIndex Index of next leg
   * @return false if no current/next leg was found
   */
  bool getRouteDistances(float *distFromStart, float *distToDest,
                         float *nextLegDistance = nullptr, float *crossTrackDistance = nullptr) const;
  float getDistanceFromStart(const atools::geo::Pos& pos) const;

  /* Ignores approach objects */
  int getNearestRouteLegResult(const atools::geo::Pos& pos, atools::geo::LineDistance& lineDistanceResult,
                               bool ignoreNotEditable) const;

  const RouteLeg& getStartAfterProcedure() const;
  const RouteLeg& getDestinationBeforeProcedure() const;

  int getActiveLegIndex() const
  {
    return activeLeg;
  }

  /* Get active leg or null if this is none */
  const RouteLeg *getActiveLeg() const;

  /* Give next procedure leg instead of route leg if approaching one */
  const RouteLeg *getActiveLegCorrected(bool *corrected = nullptr) const;

  /* Currently flying missed approach */
  bool isActiveMissed() const;

  /* Corrected methods replace the current leg with the initial fix
   * if one follows between route and transition/approach.  */
  int getActiveLegIndexCorrected(bool *corrected = nullptr) const;

  /* At the end of the route and beyond */
  bool isPassedLastLeg() const;

  /* Get top of descent position based on the option setting (default is 3 nm per 1000 ft) */
  atools::geo::Pos getTopOfDescent() const;

  /* Distance from TOD to destination in nm */
  float getTopOfDescentFromDestination() const;
  float getTopOfDescentFromStart() const;

  /* Total route distance in nautical miles */
  float getTotalDistance() const
  {
    return totalDistance;
  }

  void setTotalDistance(float value)
  {
    totalDistance = value;
  }

  const atools::fs::pln::Flightplan& getFlightplan() const
  {
    return flightplan;
  }

  atools::fs::pln::Flightplan& getFlightplan()
  {
    return flightplan;
  }

  /* Value in flight plan is stored in local unit */
  float getCruisingAltitudeFeet() const;

  void setFlightplan(const atools::fs::pln::Flightplan& value)
  {
    flightplan = value;
  }

  /* Get nearest flight plan leg to given screen position xs/ys. */
  void getNearest(const CoordinateConverter& conv, int xs, int ys, int screenDistance,
                  map::MapSearchResult& mapobjects, QList<proc::MapProcedurePoint>& procPoints,
                  bool includeProcedure) const;

  /* @return true if departure is an airport and parking is set */
  bool hasDepartureParking() const;

  /* @return true if departure is an airport and a start position is set */
  bool hasDepartureStart() const;

  /* @return true if flight plan has no departure, no destination and no intermediate waypoints */
  bool isFlightplanEmpty() const;

  /* @return true if departure is an airport */
  bool hasValidDeparture() const;

  /* @return true if destination is an airport */
  bool hasValidDestination() const;

  /* @return true if departure start position is a helipad */
  bool hasDepartureHelipad() const;

  /* @return true if has intermediate waypoints */
  bool hasEntries() const;

  /* @return true if it has at least two waypoints */
  bool canCalcRoute() const;

  /* Get a new number for a user waypoint for automatic naming */
  int getNextUserWaypointNumber() const;

  /* Index from 0 (departure) to size() -1 */
  bool canEditLeg(int index) const;

  /* Index from 0 (departure) to size() -1 */
  bool canEditPoint(int index) const;

  bool hasAnyProcedure() const
  {
    return hasAnyArrivalProcedure() || hasAnyDepartureProcedure() || hasAnyStarProcedure();
  }

  bool hasAnyArrivalProcedure() const
  {
    return !arrivalLegs.isEmpty();
  }

  bool hasTransitionProcedure() const
  {
    return !arrivalLegs.transitionLegs.isEmpty();
  }

  bool hasAnyStarProcedure() const
  {
    return !starLegs.isEmpty();
  }

  bool hasAnyDepartureProcedure() const
  {
    return !departureLegs.isEmpty();
  }

  /* Assign and update internal indexes for approach legs. Depending if legs are type SID, STAR,
   * transition or approach they are added at the end of start of the route
   *  call updateProcedureLegs after setting */
  void setArrivalProcedureLegs(const proc::MapProcedureLegs& legs)
  {
    arrivalLegs = legs;
  }

  void setStarProcedureLegs(const proc::MapProcedureLegs& legs)
  {
    starLegs = legs;
  }

  void setDepartureProcedureLegs(const proc::MapProcedureLegs& legs)
  {
    departureLegs = legs;
  }

  /* Insert legs of procedures into flight plan and update all offsets and indexes */
  void updateProcedureLegs(FlightplanEntryBuilder *entryBuilder);

  void removeRouteLegs();

  /* Does not delete flight plan properties */
  void clearProcedureLegs(proc::MapProcedureTypes type);

  /* Deletes flight plan properties too */
  void removeProcedureLegs();
  void removeProcedureLegs(proc::MapProcedureTypes type);

  /* Removes duplicate waypoints when transitioning from route to procedure and vice versa */
  void removeDuplicateRouteLegs();

  /* Needed to activate missed approach sequencing or not depending on visibility state */
  void setShownMapFeatures(map::MapObjectTypes types)
  {
    shownTypes = types;
  }

  const atools::geo::Rect& getBoundingRect() const
  {
    return boundingRect;
  }

  const atools::geo::Pos& getPositionAt(int i) const
  {
    return at(i).getPosition();
  }

  /* Update distance, course, bounding rect and total distance for route map objects.
   *  Also calculates maximum number of user points. */
  void updateAll();

  /* Set active leg and update all internal distances */
  void setActiveLeg(int value);

  /* Set to no active leg */
  void resetActive();

  /* All couse values in the route are true since no navaid having magvar was found */
  bool isTrueCourse() const
  {
    return trueCourse;
  }

  /* true if type is airport at the given index and is after an arrival procedure (approach and transition) */
  bool isAirportAfterArrival(int index);

  /* Get approach and transition in one legs struct */
  const proc::MapProcedureLegs& getArrivalLegs() const
  {
    return arrivalLegs;
  }

  /* Get STAR legs only */
  const proc::MapProcedureLegs& getStarLegs() const
  {
    return starLegs;
  }

  /* Get SID legs only */
  const proc::MapProcedureLegs& getDepartureLegs() const
  {
    return departureLegs;
  }

  /* Index of first transition and/or approach leg in the route */
  int getArrivalLegsOffset() const
  {
    return arrivalLegsOffset;
  }

  /* Index of first SID leg in the route */
  int getDepartureLegsOffset() const
  {
    return departureLegsOffset;
  }

  /* Index of first STAR leg in the route */
  int getStarLegsOffset() const
  {
    return starLegsOffset;
  }

  /* Pull only the needed methods in public space to have control over it */
  using QList<RouteLeg>::const_iterator;
  using QList<RouteLeg>::begin;
  using QList<RouteLeg>::end;
  using QList<RouteLeg>::at;
  using QList<RouteLeg>::first;
  using QList<RouteLeg>::last;
  using QList<RouteLeg>::size;
  using QList<RouteLeg>::isEmpty;
  using QList<RouteLeg>::clear;
  using QList<RouteLeg>::append;
  using QList<RouteLeg>::prepend;
  using QList<RouteLeg>::insert;
  using QList<RouteLeg>::replace;
  using QList<RouteLeg>::move;
  using QList<RouteLeg>::removeAt;
  using QList<RouteLeg>::removeLast;
  using QList<RouteLeg>::operator[];

private:
  void clearFlightplanProcedureProperties(proc::MapProcedureTypes type);

  /* Calculate all distances and courses for route map objects */
  void updateDistancesAndCourse();
  void updateBoundingRect();

  /* Update and calculate magnetic variation for all route map objects */
  void updateMagvar();

  /* Assign index and pointer to flight plan for all objects */
  void updateIndicesAndOffsets();

  /* Get a position along the route. Pos is invalid if not along. distFromStart in nm */
  atools::geo::Pos positionAtDistance(float distFromStartNm) const;

  /* Get indexes to nearest approach or route leg and cross track distance to the nearest ofthem in nm */
  void copy(const Route& other);
  void nearestAllLegIndex(const map::PosCourse& pos, float& crossTrackDistanceMeter, int& index) const;
  bool isSmaller(const atools::geo::LineDistance& dist1, const atools::geo::LineDistance& dist2, float epsilon);
  void eraseProcedureLegs(proc::MapProcedureTypes type);

  bool trueCourse = false;

  atools::geo::Rect boundingRect;
  /* Nautical miles not including missed approach */
  float totalDistance = 0.f;
  atools::fs::pln::Flightplan flightplan;
  proc::MapProcedureLegs arrivalLegs, starLegs, departureLegs;
  map::MapObjectTypes shownTypes;

  int activeLeg = map::INVALID_INDEX_VALUE;
  atools::geo::LineDistance activeLegResult;
  map::PosCourse activePos;
  int departureLegsOffset = map::INVALID_INDEX_VALUE, starLegsOffset = map::INVALID_INDEX_VALUE,
      arrivalLegsOffset = map::INVALID_INDEX_VALUE;

  int adjustedActiveLeg() const;

};

QDebug operator<<(QDebug out, const Route& route);

#endif // LITTLENAVMAP_ROUTE_H