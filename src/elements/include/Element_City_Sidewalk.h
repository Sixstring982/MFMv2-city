/*                                              -*- mode:C++ -*-
  Element_City_Sidewalk.h Building border for City simulation
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file Element_City_Sidewalk.h Building border for City simulation
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_CITY_SIDEWALK_H
#define ELEMENT_CITY_SIDEWALK_H

#include "CityConstants.h"
#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "Dirs.h"
#include "itype.h"
#include "P3Atom.h"
#include "Element_City_Park.h"
#include "WindowScanner.h"

namespace MFM
{
#define CITY_VERSION 1

  template<class CC>
  class Element_City_Sidewalk : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;

    enum
    {
      R = P::EVENT_WINDOW_RADIUS,
      BITS = P::BITS_PER_ATOM,

      BUILDING_FLAG_POS = P3Atom<P>::ATOM_FIRST_STATE_BIT,
      BUILDING_FLAG_LEN = 1,

      BUILDING_TIMER_POS = BUILDING_FLAG_POS + BUILDING_FLAG_LEN,
      BUILDING_TIMER_LEN = 10,

      MAX_TIMER_VALUE = (1 << BUILDING_TIMER_LEN) - 1,

      /*
       * Split this into two bit fields becase accessing must be done
       * in chuncks of 32 bits
       */
      BUILDING_DIST1_POS = BUILDING_FLAG_POS + BUILDING_FLAG_LEN,
      BUILDING_DIST1_LEN = CityConstants::CITY_BUILDING_COUNT,

      BUILDING_DIST2_POS = BUILDING_DIST1_POS + BUILDING_DIST1_LEN,
      BUILDING_DIST2_LEN = CityConstants::CITY_BUILDING_COUNT
    };

    typedef
    BitField<BitVector<BITS>, VD::U32, BUILDING_FLAG_LEN, BUILDING_FLAG_POS> AFBuildingFlag;
    typedef
    BitField<BitVector<BITS>, VD::U32, BUILDING_TIMER_LEN, BUILDING_TIMER_POS> AFBuildingTimer;
    typedef
    BitField<BitVector<BITS>, VD::U32, BUILDING_DIST1_LEN, BUILDING_DIST2_POS> AFBuildingDist1;
    typedef
    BitField<BitVector<BITS>, VD::U32, BUILDING_DIST2_LEN, BUILDING_DIST2_POS> AFBuildingDist2;

    void SetReadyToBuild(T& us) const
    {
      AFBuildingFlag::Write(this->GetBits(us), 1);
      ClearBuildingCounts(us);
    }

    bool IsReadyToBuild(const T& us) const
    {
      return AFBuildingFlag::Read(this->GetBits(us)) != 0;
    }

    u32 GetBuildingTimer(const T& us) const
    {
      return AFBuildingTimer::Read(this->GetBits(us));
    }

    void SetBuildingTimer(T& us, const u32 val) const
    {
      AFBuildingTimer::Write(this->GetBits(us), val);
    }

    void TickBuildingTimer(T& us) const
    {
      u32 timerVal = GetBuildingTimer(us);
      if(timerVal != MAX_TIMER_VALUE)
      {
        SetBuildingTimer(us, GetBuildingTimer(us) + 1);
      }
      else
      {
        SetReadyToBuild(us);
      }
    }

    void ClearBuildingCounts(T& us) const
    {
      for(u32 i = 0; i < CityConstants::CITY_BUILDING_COUNT; i++)
      {
        SetBuildingCount(us, i, 3);
      }
    }

    void SetBuildingCount(T& us, u32 buildingType, u32 buildingCount) const
    {
      if(buildingCount > 3)
      {
        FAIL(ILLEGAL_ARGUMENT);
      }

      u32 topBit = (buildingCount & 2) >> 1;
      u32 botBit = (buildingCount & 1);

      u32 topWord = AFBuildingDist1::Read(this->GetBits(us));
      u32 botWord = AFBuildingDist2::Read(this->GetBits(us));

      if(topBit)
      {
        topWord |= (1 << buildingType);
      }
      else
      {
        topWord &= ~(1 << buildingType);
      }

      if(botBit)
      {
        botWord |= (1 << buildingType);
      }
      else
      {
        botWord &= ~(1 << buildingType);
      }

      AFBuildingDist1::Write(this->GetBits(us), topWord);
      AFBuildingDist2::Write(this->GetBits(us), botWord);
    }

   public:
    u32 GetBuildingCount(const T& us, u32 buildingType) const
    {
      u32 ans = 0;
      u32 top = AFBuildingDist1::Read(this->GetBits(us));
      u32 bot = AFBuildingDist2::Read(this->GetBits(us));

      ans = ((top & (1 << buildingType)) > 0);
      ans <<= 1;

      ans |= ((bot & (1 << buildingType)) > 0);

      if(ans < 0 || ans > 3)
      {
        FAIL(ILLEGAL_STATE);
      }

      return ans;
    }

    static Element_City_Sidewalk THE_INSTANCE;

    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    Element_City_Sidewalk() :
      Element<CC>(MFM_UUID_FOR("CitySidewalk", CITY_VERSION))
    {
      Element<CC>::SetAtomicSymbol("Sw");
      Element<CC>::SetName("City Sidewalk");
    }

    virtual u32 PercentMovable(const T& you, const T& me, const SPoint& offset) const
    {
      return 0;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff808080;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff404040;
    }

    virtual u32 LocalPhysicsColor(const T& atom, u32 selector) const
    {
      if(IsReadyToBuild(atom))
      {
        return 0xff707070;
      }
      else
      {
        return Element<CC>::PhysicsColor();
      }
    }

    virtual const char* GetDescription() const
    {
      return "Building border element for city simulation.";
    }

    virtual u32 Diffusability(EventWindow<CC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return nowAt.Equals(maybeAt)?Element<CC>::COMPLETE_DIFFUSABILITY:0;
    }

    void DoTimerBehavior(EventWindow<CC>& window) const
    {
      T newMe = window.GetCenterAtom();
      TickBuildingTimer(newMe);

      window.SetCenterAtom(newMe);
    }

    u32 GetBuildingType() const;

    u32 GetStreetType() const;

    void DoBuildingBehavior(EventWindow<CC>& window) const;

    void UpdateBuildingCounts(EventWindow<CC>& window) const;

    void DoParkBehavior(EventWindow<CC>& window) const
    {
      WindowScanner<CC> scanner(window);
      const u32 types[] =
      {
        TYPE(),
        Element_City_Park<CC>::THE_INSTANCE.GetType()
      };
      SPoint not_used;
      if(scanner.FindRandomInVonNeumann(types[0], not_used) +
         scanner.FindRandomInVonNeumann(types[1], not_used) == 4)
      {
        window.SetCenterAtom(Element_City_Park<CC>::THE_INSTANCE.GetDefaultAtom());
      }
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      WindowScanner<CC> scanner(window);
      SPoint not_used;
      DoParkBehavior(window);

      if(IsReadyToBuild(window.GetCenterAtom()))
      {
        DoBuildingBehavior(window);
        UpdateBuildingCounts(window);
      }
      else
      {
        DoTimerBehavior(window);

        if(scanner.FindRandomInVonNeumann(GetBuildingType(), not_used) > 0)
        {
          /* Building bordering us, meaning ticking the timer is
             essentially useless. */
          T newMe = window.GetCenterAtom();
          SetReadyToBuild(newMe);
          window.SetCenterAtom(newMe);
        }
      }
    }
  };

  template <class CC>
  Element_City_Sidewalk<CC> Element_City_Sidewalk<CC>::THE_INSTANCE;
}

#include "Element_City_Sidewalk.tcc"

#endif /* ELEMENT_CITY_SIDEWALK_H */
