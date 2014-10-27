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

  /* This returns Dirs::DIR_COUNT if this is a screwy intersection and the
   * waiting car should just be routed to a random existing street. */
  template <class CC>
  Dir Element_City_Intersection<CC>::FindBestRoute(EventWindow<CC>& window,
                                                   u32 destinationType,
                                                   Dir comingFrom) const
  {
    SPoint roads[4]; /* N E S W */
    u32 roadFitness[4] = { 100 };

    Dir d = Dirs::NORTH;
    for(u32 i = 0; i < 4; i++)
    {
      d = Dirs::CWDir(Dirs::CWDir(d));
      Dirs::FillDir(roads[i], d);

      if(window.IsLiveSite(roads[i]))
      {
        if(window.GetRelativeAtom(roads[i]).GetType() == GetStreetType() ||
           window.GetRelativeAtom(roads[i]).GetType() == GetCarType())
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
              return Dirs::DIR_COUNT;
            }
            roadFitness[i] -=
            Element_City_Sidewalk<CC>::THE_INSTANCE.GetBuildingCount(
              window.GetRelativeAtom(edgeSidewalk), destinationType);
          }
        }
      }
    }

    u32 bestRoad = 0;
    bool foundRoad = false;
    for(u32 i = 1; i < 4; i++)
    {
      if(roadFitness[i] < roadFitness[bestRoad])
      {
        bestRoad = i;
        foundRoad = true;
      }
    }

    if(!foundRoad)
    {
      return Dirs::DIR_COUNT;
    }
    return (Dir)(bestRoad * 2);
  }

  template <class CC>
  void Element_City_Intersection<CC>::DoRouting(EventWindow<CC>& window) const
  {
    WindowScanner<CC> scanner(window);
    SPoint carToMove;

    if(scanner.FindRandomInVonNeumann(GetCarType(), carToMove) > 0)
    {
      Dir bestRoute = FindBestRoute(window,
                      Element_City_Car<CC>::THE_INSTANCE.
                                    GetDestType(window.GetRelativeAtom(carToMove)),
                      Element_City_Car<CC>::THE_INSTANCE.
                                    GetDirection(window.GetRelativeAtom(carToMove)));

      if(bestRoute == Dirs::DIR_COUNT)
      {
        /* Let's pick a random dir */
        bestRoute = window.GetRandom().Create(Dirs::DIR_COUNT);
        if(bestRoute & 1)
        {
          bestRoute = Dirs::CWDir(bestRoute);
        }
      }

      if(bestRoute != Dirs::DIR_COUNT)
      {
        SPoint bestDirPt;
        Dirs::FillDir(bestDirPt, bestRoute);

        if(window.GetRelativeAtom(bestDirPt).GetType() == GetCarType())
        {

          /* If it's us, just change direction and move on. */
          if(bestDirPt.Equals(carToMove))
          {
            T uTurnCar = window.GetRelativeAtom(carToMove);
            Element_City_Car<CC>::THE_INSTANCE.
                SetDirection(uTurnCar,
                             Dirs::OppositeDir(
                               Element_City_Car<CC>::THE_INSTANCE.
                                   GetDirection(uTurnCar)));
            window.SetRelativeAtom(carToMove, uTurnCar);
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
        }
      }
    }
  }

  template <class CC>
  void Element_City_Intersection<CC>::InitializeIntersection(T& atom,
                                                             EventWindow<CC>& window) const
  {
    Random& rand = window.GetRandom();
    Dir d;
    SPoint offset;
    for(u32 i = 0; i < Dirs::DIR_COUNT; i++)
    {
      d = (Dir)i;
      Dirs::FillDir(offset, d);

      if(window.GetRelativeAtom(offset).GetType() ==
          Element_Empty<CC>::THE_INSTANCE.GetType())
      {
        if(rand.OneIn(m_streetCreateOdds.GetValue()) && !(d & 1))
        {
          Element_City_Street<CC>& st = Element_City_Street<CC>::THE_INSTANCE;
          T newStreet = st.GetDefaultAtom();
          st.SetDirection(newStreet, d);
          window.SetRelativeAtom(offset, newStreet);
        }
        else
        {
          T newSidewalk = Element_City_Sidewalk<CC>::THE_INSTANCE.GetDefaultAtom();
          window.SetRelativeAtom(offset, newSidewalk);
        }
      }
    }
  }
}
