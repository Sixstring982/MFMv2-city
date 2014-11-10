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

  /* This returns Dirs::DIR_COUNT if this is a screwy intersection and the
   * waiting car should just be routed to a random existing street. */
  template <class CC>
  Dir Element_City_Intersection<CC>::FindBestRoute(EventWindow<CC>& window,
                                                   u32 destinationType,
                                                   Dir comingFrom) const
  {
    SPoint roads[4]; /* N E S W */
    u32 roadFitness[4] = { 100, 100, 100, 100 };

    Dir d = Dirs::NORTH;
    for(u32 i = 0; i < 4; i++)
    {
      Dirs::FillDir(roads[i], d);

      if(window.IsLiveSite(roads[i]))
      {
        if(window.GetRelativeAtom(roads[i]).GetType() != GetStreetType() &&
           window.GetRelativeAtom(roads[i]).GetType() != GetCarType())
        {
          roadFitness[i] += 100;
        }
        else
        {
          for(u32 j = 0; j < 2; j++)
          {
            SPoint edgeSidewalk;
            if(j == 0)
            {
              Dirs::FillDir(edgeSidewalk, Dirs::CWDir(Dirs::CWDir(d)));
            }
            else
            {
              Dirs::FillDir(edgeSidewalk, Dirs::CCWDir(Dirs::CCWDir(d)));
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
      d = Dirs::CWDir(Dirs::CWDir(d));
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
        dirsOfMin[bestCount++] = i * 2;
      }
    }

    Dir bestDir = dirsOfMin[window.GetRandom().Create(bestCount)];

    /* If we're going to U-turn the car, let's tell him to go randomly. */
    if(bestDir == Dirs::OppositeDir(comingFrom))
    {
      u32 odds = window.GetRandom().Create(100);

      if(odds < 1)
      {
        bestDir = Dirs::OppositeDir(comingFrom);
      }
      else if(odds < 34 && IsRelDirCarOrStreet(window,
                                               Dirs::CCWDir(Dirs::CCWDir(comingFrom))))
      {
        bestDir = Dirs::CCWDir(Dirs::CCWDir(comingFrom));
      }
      else if(odds < 67 && IsRelDirCarOrStreet(window,
                                               Dirs::OppositeDir(comingFrom)))
      {
        bestDir = comingFrom;
      }
      else if(IsRelDirCarOrStreet(window, Dirs::CWDir(Dirs::CWDir(comingFrom))))
      {
        bestDir = Dirs::CWDir(Dirs::CWDir(comingFrom));
      }
    }

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

    if(scanner.FindRandomInVonNeumann(GetCarType(), carToMove) > 0)
    {
      /* If there's nowhere else to go, turn around. */
      if(scanner.CountVonNeumannNeighbors(GetCarType()) +
         scanner.CountVonNeumannNeighbors(GetStreetType()) == 1)
      {
        UTurnCar(window, carToMove);
        return;
      }

      Dir bestRoute = FindBestRoute(window,
                      Element_City_Car<CC>::THE_INSTANCE.
                                    GetDestType(window.GetRelativeAtom(carToMove)),
                      Element_City_Car<CC>::THE_INSTANCE.
                                    GetDirection(window.GetRelativeAtom(carToMove)));
      SPoint bestDirPt;
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
        Element_City_Car<CC>::THE_INSTANCE.SetDirection(movingCar,
                                                        Dirs::FromOffset(bestDirPt));

        T stoppedCar = window.GetRelativeAtom(bestDirPt);
        Element_City_Car<CC>::THE_INSTANCE.SetDirection(stoppedCar,
                                                        Dirs::OppositeDir(Dirs::FromOffset(carToMove)));

        window.SetRelativeAtom(bestDirPt, movingCar);
        window.SetRelativeAtom(carToMove, stoppedCar);
      }
      else if(window.GetRelativeAtom(bestDirPt).GetType() == GetStreetType())
      {
        /* Gotta be a street. We need to reconstruct the street with its correct direction. */

        T movingCar = window.GetRelativeAtom(carToMove);
        Element_City_Car<CC>::THE_INSTANCE.SetDirection(movingCar,
                                                        Dirs::FromOffset(bestDirPt));

        T street = window.GetRelativeAtom(bestDirPt);
        Element_City_Street<CC>::THE_INSTANCE.SetDirection(street,
                                                           Dirs::OppositeDir(Dirs::FromOffset(carToMove)));

        window.SetRelativeAtom(bestDirPt, movingCar);
        window.SetRelativeAtom(carToMove, street);
      }
      else
      {
        /* Crap, didn't find anything good. Screw it, find something next time.*/
        /* We gotta do something though. Let's move that car randomly. */
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
    Dir dirs[4] = {Dirs::NORTH, Dirs::EAST, Dirs::SOUTH, Dirs::WEST};
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
