/* -*- C++ -*- */

#include "Element_City_Street.h"
#include "Element_City_Sidewalk.h"
#include "Element_City_Car.h"

namespace MFM
{
  template <class CC>
  u32 Element_City_Intersection<CC>::GetStreetType() const
  {
    return Element_City_Street<CC>::THE_INSTANCE.GetType();
  }

  template <class CC>
  u32 Element_City_Intersection<CC>::GetCarType() const
  {
    return Element_City_Car<CC>::THE_INSTANCE.GetType();
  }

  template <class CC>
  u32 Element_City_Intersection<CC>::GetSidewalkType() const
  {
    return Element_City_Sidewalk<CC>::THE_INSTANCE.GetType();
  }

  template <class CC>
  u32 Element_City_Intersection<CC>::IsRelDirCarOrStreet(EventWindow<CC> window,
                                                         Dir d) const
  {
    SPoint dp;
    Dirs::FillDir(dp, d);
    const T& atom = window.GetRelativeAtom(dp);

    return atom.GetType() == GetCarType() || atom.GetType() == GetStreetType();
  }

  template <class CC>
  bool Element_City_Intersection<CC>::CanalIsValid(EventWindow<CC>& window,
                                                   u32 destType) const
  {
    Dir canal = GetCanalDir(window.GetCenterAtom(), destType);
    SPoint canalPt;
    Dirs::FillDir(canalPt, canal);
    return window.GetRelativeAtom(canalPt).GetType() == GetStreetType();
  }

  template <class CC>
  Dir Element_City_Intersection<CC>::FindRandomRoute(EventWindow<CC>& window) const
  {
    return (Dir)((window.GetRandom().Create(4)) * 2);
  }

  template <class CC>
  Dir Element_City_Intersection<CC>::FindBestRouteCanal(EventWindow<CC>& window,
                                                        u32 destinationType,
                                                        Dir comingFrom) const
  {
    Dir canal = GetCanalDir(window.GetCenterAtom(), destinationType);

    /* First, see if the canal will help get him in the right
     * direction.  If the canal points in a direction that he isn't
     * coming from, route him towards the canal.
     */

    if (CanalIsValid(window, destinationType) &&
        canal != Dirs::OppositeDir(comingFrom))
    {
      return canal;
    }


    /*
     * The canal seems to have been wrong... Let's go ahead and find
     * another direction and set the canal accordingly.
     */

    u32 roadFitness[] = {100, 100, 100};
    SPoint roads[3];
    Dir dirs[3]; /* The non-canal directions */
    u32 i = 0;
    for (Dir d = Dirs::NORTH; d < Dirs::DIR_COUNT; d += 2)
    {
      if(d != Dirs::OppositeDir(comingFrom))
      {
        dirs[i] = d;
        Dirs::FillDir(roads[i], dirs[i]);
        i++;
      }
    }

    for(u32 i = 0; i < 3; i++)
    {
      if(window.IsLiveSite(roads[i]))
      {
        if(window.GetRelativeAtom(roads[i]).GetType() != GetStreetType() &&
           window.GetRelativeAtom(roads[i]).GetType() != GetCarType())
        {
          roadFitness[i] += 100; /* That isn't a road! */
        }
        else
        {
          for(u32 j = 0; j < 2; j++)
          {
            SPoint edgeSidewalk;
            if(j == 0)
            {
              Dirs::FillDir(edgeSidewalk, Dirs::CWDir(Dirs::CWDir(dirs[i])));
            }
            else
            {
              Dirs::FillDir(edgeSidewalk, Dirs::CCWDir(Dirs::CCWDir(dirs[i])));
            }

            edgeSidewalk = edgeSidewalk + roads[i];
            if(window.GetRelativeAtom(edgeSidewalk).GetType() !=
               Element_City_Sidewalk<CC>::THE_INSTANCE.GetType())
            {
              roadFitness[i] += 100; /* Don't want to go that way! */
            }
            else
            {
              if(Element_City_Sidewalk<CC>::THE_INSTANCE.
                 IsReadyToBuild(window.GetRelativeAtom(edgeSidewalk)))
              {
                roadFitness[i] -=
                3 - (
                Element_City_Sidewalk<CC>::THE_INSTANCE.
                GetBuildingCount(window.
                                 GetRelativeAtom(edgeSidewalk), destinationType));
              }
              else
              {
                roadFitness[i]++;
              }
            }
          }
        }
      }
    }

    u32 bestDirValue = MIN(roadFitness[0],
                           MIN(roadFitness[1], roadFitness[2]));
    u32 bestCount = 0;
    Dir dirsOfMin[3];

    for(u32 i = 0; i < 3; i++)
    {
      if(roadFitness[i] == bestDirValue)
      {
        dirsOfMin[bestCount++] = dirs[i];
      }
    }

    Dir bestDir = dirsOfMin[window.GetRandom().Create(bestCount)];

    /* Now that we have this, reassign the canal. */
    T newAtom = window.GetCenterAtom();
    SetCanalDir(newAtom, destinationType, bestDir);
    window.SetCenterAtom(newAtom);

    return bestDir;
  }


  template <class CC>
  Dir Element_City_Intersection<CC>::FindBestRouteStandard(EventWindow<CC>& window,
                                                           u32 destinationType,
                                                           Dir comingFrom) const
  {
    u32 roadFitness[] = {100, 100, 100, 100};
    SPoint roads[4];
    Dir dirs[4]; /* The non-canal directions */
    u32 i = 0;
    for (Dir d = Dirs::NORTH; d < Dirs::DIR_COUNT; d += 2)
    {
      dirs[i] = d;
      Dirs::FillDir(roads[i], dirs[i]);
      i++;
    }

    for(u32 i = 0; i < 4; i++)
    {
      if(window.IsLiveSite(roads[i]))
      {
        if(window.GetRelativeAtom(roads[i]).GetType() != GetStreetType() &&
           window.GetRelativeAtom(roads[i]).GetType() != GetCarType())
        {
          roadFitness[i] += 100; /* That isn't a road! */
        }
        else
        {
          for(u32 j = 0; j < 2; j++)
          {
            SPoint edgeSidewalk;
            if(j == 0)
            {
              Dirs::FillDir(edgeSidewalk, Dirs::CWDir(Dirs::CWDir(dirs[i])));
            }
            else
            {
              Dirs::FillDir(edgeSidewalk, Dirs::CCWDir(Dirs::CCWDir(dirs[i])));
            }

            edgeSidewalk = edgeSidewalk + roads[i];
            if(window.GetRelativeAtom(edgeSidewalk).GetType() !=
               Element_City_Sidewalk<CC>::THE_INSTANCE.GetType())
            {
              roadFitness[i] += 100; /* Don't want to go that way! */
            }
            else
            {
              if(Element_City_Sidewalk<CC>::THE_INSTANCE.
                 IsReadyToBuild(window.GetRelativeAtom(edgeSidewalk)))
              {
                roadFitness[i] -=
                3 - (
                Element_City_Sidewalk<CC>::THE_INSTANCE.
                GetBuildingCount(window.
                                 GetRelativeAtom(edgeSidewalk), destinationType));
              }
              else
              {
                roadFitness[i]++;
              }
            }
          }
        }
      }
    }

    u32 bestDirValue = MIN(roadFitness[0],
                           MIN(roadFitness[1],
                               MIN(roadFitness[2], roadFitness[3])));
    u32 bestCount = 0;
    Dir dirsOfMin[4];

    for(u32 i = 0; i < 4; i++)
    {
      if(roadFitness[i] == bestDirValue)
      {
        dirsOfMin[bestCount++] = dirs[i];
      }
    }

    Dir bestDir = dirsOfMin[window.GetRandom().Create(bestCount)];

    return bestDir;
  }

  template <class CC>
  void Element_City_Intersection<CC>::UTurnCar(EventWindow<CC>& window,
                                               SPoint& carAt) const
  {
    T uTurnCar = window.GetRelativeAtom(carAt);
    Element_City_Car<CC>::THE_INSTANCE.
    SetDirection(uTurnCar,
                 Dirs::OppositeDir(
                   Element_City_Car<CC>::THE_INSTANCE.
                   GetDirection(uTurnCar)));
    window.SetRelativeAtom(carAt, uTurnCar);
    return;
  }

  template <class CC>
  void Element_City_Intersection<CC>::DoRouting(EventWindow<CC>& window) const
  {
    WindowScanner<CC> scanner(window);
    SPoint carToMove;
    u32 streetsAndCars;

    if(scanner.FindRandomInVonNeumann(GetCarType(), carToMove) > 0)
    {
      streetsAndCars =
      scanner.CountVonNeumannNeighbors(GetCarType()) +
      scanner.CountVonNeumannNeighbors(GetStreetType());
      /* If there's nowhere else to go, turn around. */
      if (streetsAndCars == 1)
      {
        UTurnCar(window, carToMove);
        return;
      }

      Dir bestRoute = Dirs::NORTH;
      SPoint bestDirPt;

      /* Stupid two-sidewalk corner cases */
      if (streetsAndCars == 2)
      {
        Dir start = Dirs::FromOffset(carToMove);
        SPoint pt;
        bool found = true;
        for(u32 i = 0; i < 3; i++)
        {
          start = Dirs::CWDir(Dirs::CWDir(start));
          Dirs::FillDir(pt, start);
          if ((window.GetRelativeAtom(pt).GetType() == GetCarType()) ||
              (window.GetRelativeAtom(pt).GetType() == GetStreetType()))
          {
            bestRoute = start;
            bestDirPt = pt;
            break;
          }
        }
        if (!found)
        {
          FAIL(ILLEGAL_STATE);
        }
        else if(window.GetRelativeAtom(pt).GetType() == GetStreetType())
        {
          /* It's a street, but we need to be careful. Let's give the
           * car the same direction that the street has. */
          T movingCar = window.GetRelativeAtom(carToMove);
          Dir moveFrom = Element_City_Car<CC>::THE_INSTANCE.GetDirection(movingCar);

          T street = window.GetRelativeAtom(bestDirPt);
          Dir streetDir = Element_City_Street<CC>::THE_INSTANCE.GetDirection(street);
          Element_City_Street<CC>::THE_INSTANCE.SetDirection(street, moveFrom);
          if (streetDir == Dirs::OppositeDir(start))
          {
            streetDir = start; /* Don't point the car at the intersection again */
          }
          Element_City_Car<CC>::THE_INSTANCE.SetDirection(movingCar, streetDir);

          window.SetRelativeAtom(bestDirPt, movingCar);
          window.SetRelativeAtom(carToMove, street);
          return;
        }
        /* Otherwise, it's a car and the normal behavior is sufficient. */
      }
      else
      {

#ifdef RANDOM_ROUTING
        bestRoute = FindRandomRoute(window);
#elif defined CANAL_ROUTING
        bestRoute = FindBestRouteCanal(
          window,
          Element_City_Car<CC>::THE_INSTANCE.
          GetDestType(window.GetRelativeAtom(carToMove)),
          Element_City_Car<CC>::THE_INSTANCE.
          GetDirection(window.GetRelativeAtom(carToMove)));
#else
        bestRoute = FindBestRouteStandard(
          window,
          Element_City_Car<CC>::THE_INSTANCE.
          GetDestType(window.GetRelativeAtom(carToMove)),
          Element_City_Car<CC>::THE_INSTANCE.
          GetDirection(window.GetRelativeAtom(carToMove)));
#endif

      }
      Dirs::FillDir(bestDirPt, bestRoute);

      if(window.GetRelativeAtom(bestDirPt).GetType() == GetCarType())
      {
        /* If it's us, just change direction and move on. */
        if(bestDirPt.Equals(carToMove))
        {
          UTurnCar(window, carToMove);
          return;
        }

        /* If it's a car we can just swap, but we need to redirect both cars. */

        T movingCar = window.GetRelativeAtom(carToMove);
        Dir moveFrom = Element_City_Car<CC>::THE_INSTANCE.GetDirection(movingCar);
        Element_City_Car<CC>::THE_INSTANCE.SetDirection(movingCar, bestRoute);

        T stoppedCar = window.GetRelativeAtom(bestDirPt);
        Element_City_Car<CC>::THE_INSTANCE.SetDirection(stoppedCar, moveFrom);


        window.SetRelativeAtom(bestDirPt, movingCar);
        window.SetRelativeAtom(carToMove, stoppedCar);
      }
      else if(window.GetRelativeAtom(bestDirPt).GetType() == GetStreetType())
      {
        /* Gotta be a street. We need to reconstruct the street with its correct direction. */

        T movingCar = window.GetRelativeAtom(carToMove);
        Dir moveFrom = Element_City_Car<CC>::THE_INSTANCE.GetDirection(movingCar);
        Element_City_Car<CC>::THE_INSTANCE.SetDirection(movingCar, bestRoute);

        T street = window.GetRelativeAtom(bestDirPt);
        Element_City_Street<CC>::THE_INSTANCE.
                                 SetDirection(street, moveFrom);

        window.SetRelativeAtom(bestDirPt, movingCar);
        window.SetRelativeAtom(carToMove, street);
      }
    }
  }

  template <class CC>
  void Element_City_Intersection<CC>::CreateStreetFromEmpty(EventWindow<CC>& window, Dir d) const
  {
    SPoint pt;
    Dirs::FillDir(pt, d);
    if(window.GetRelativeAtom(pt).GetType() ==
       Element_Empty<CC>::THE_INSTANCE.GetType())
    {
      Element_City_Street<CC>& st = Element_City_Street<CC>::THE_INSTANCE;
      T newStreet = st.GetDefaultAtom();
      st.SetDirection(newStreet, d);
      window.SetRelativeAtom(pt, newStreet);
    }
  }

  template <class CC>
  void Element_City_Intersection<CC>::CreateSidewalkFromEmpty(EventWindow<CC>& window, Dir d) const
  {
    SPoint pt;
    Dirs::FillDir(pt, d);
    if(window.GetRelativeAtom(pt).GetType() ==
       Element_Empty<CC>::THE_INSTANCE.GetType())
    {
      T newSidewalk = Element_City_Sidewalk<CC>::THE_INSTANCE.GetDefaultAtom();
      window.SetRelativeAtom(pt, newSidewalk);
    }
  }

  template <class CC>
  void Element_City_Intersection<CC>::InitializeIntersection(T& atom,
                                                             EventWindow<CC>& window) const
  {
    Random& rand = window.GetRandom();
    Dir dirs[4] = { Dirs::NORTH, Dirs::EAST, Dirs::SOUTH, Dirs::WEST };
    SPoint offset;

    /* Shuffle dirs */
    for(u32 i = 0; i < 28; i++)
    {
      u32 a = rand.Create(4);
      u32 b = rand.Create(4);

      if(a != b)
      {
        Dir tmp = dirs[a];
        dirs[a] = dirs[b];
        dirs[b] = tmp;
      }
    }

    for(u32 i = 0; i < m_minCreatedStreets.GetValue(); i++)
    {
      CreateStreetFromEmpty(window, dirs[i]);
    }

    for(Dir d = Dirs::NORTH; d < Dirs::DIR_COUNT; d = (Dir)(d + 1))
    {
      CreateSidewalkFromEmpty(window, d);
    }
  }
}
