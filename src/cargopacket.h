/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file cargopacket.h Base class for cargo packets. */

#ifndef CARGOPACKET_H
#define CARGOPACKET_H

#include "core/pool_type.hpp"
#include "economy_type.h"
#include "station_type.h"
#include "order_type.h"
#include "cargo_type.h"
#include "vehicle_type.h"
#include "core/multimap.hpp"
#include "saveload/saveload_data.h"
#include <list>

/** Unique identifier for a single cargo packet. */
typedef uint32 CargoPacketID;
struct CargoPacket;

struct GoodsEntry; // forward-declare for Stage() and RerouteStalePackets()

template <class Tinst, class Tcont> class CargoList;
class StationCargoList; // forward-declare, so we can use it in VehicleCargoList.
extern const struct SaveLoad *GetCargoPacketDesc();

typedef uint32 TileOrStationID;

/**
 * Container for cargo from the same location and time.
 */
struct CargoPacket : PooledItem <CargoPacket, CargoPacketID, 1024, 0xFFF000, PT_NORMAL, true, false> {
private:
	Money feeder_share;         ///< Value of feeder pickup to be paid for on delivery of cargo.
	uint16 count;               ///< The amount of cargo in this packet.
	byte days_in_transit;       ///< Amount of days this packet has been in transit.
	CargoSource source;         ///< Source of cargo.
	StationID source_st;        ///< The station where the cargo came from first.
	TileIndex source_xy;        ///< The origin of the cargo (first station in feeder chain).
	union {
		TileOrStationID loaded_at_xy; ///< Location where this cargo has been loaded into the vehicle.
		TileOrStationID next_station; ///< Station where the cargo wants to go next.
	};

	/** The CargoList caches, thus needs to know about it. */
	template <class Tinst, class Tcont> friend class CargoList;
	friend class VehicleCargoList;
	friend class StationCargoList;
	/** We want this to be saved, right? */
	friend const struct SaveLoad *GetCargoPacketDesc();
public:
	/** Maximum number of items in a single cargo packet. */
	static const uint16 MAX_COUNT = UINT16_MAX;

	CargoPacket();
	CargoPacket (const struct Station *st, uint16 count, SourceType source_type, SourceID source_id);
	CargoPacket (uint16 count, byte days_in_transit, StationID source_st, TileIndex source_xy, TileIndex loaded_at_xy, Money feeder_share = 0);
	CargoPacket (const CargoPacket &cp, uint16 count, Money share);

	/** Destroy the packet. */
	~CargoPacket() { }

	CargoPacket *Split(uint new_size);
	void Merge(CargoPacket *cp);
	void Reduce(uint count);

	/**
	 * Try to merge another packet into this one, if the total count allows it.
	 * @param cp Packet to merge.
	 * @return Whether merging was possible (and done).
	 */
	bool TryMerge (CargoPacket *cp)
	{
		if (this->count + cp->count > MAX_COUNT) return false;
		this->Merge (cp);
		return true;
	}

	/**
	 * Sets the tile where the packet was loaded last.
	 * @param load_place Tile where the packet was loaded last.
	 */
	void SetLoadPlace(TileIndex load_place) { this->loaded_at_xy = load_place; }

	/**
	 * Sets the station where the packet is supposed to go next.
	 * @param next_station Next station the packet should go to.
	 */
	void SetNextStation(StationID next_station) { this->next_station = next_station; }

	/**
	 * Adds some feeder share to the packet.
	 * @param new_share Feeder share to be added.
	 */
	void AddFeederShare(Money new_share) { this->feeder_share += new_share; }

	/**
	 * Gets the number of 'items' in this packet.
	 * @return Item count.
	 */
	inline uint16 Count() const
	{
		return this->count;
	}

	/**
	 * Gets the amount of money already paid to earlier vehicles in
	 * the feeder chain.
	 * @return Feeder share.
	 */
	inline Money FeederShare() const
	{
		return this->feeder_share;
	}

	/**
	 * Gets part of the amount of money already paid to earlier vehicles in
	 * the feeder chain.
	 * @param part Amount of cargo to get the share for.
	 * @return Feeder share for the given amount of cargo.
	 */
	inline Money FeederShare(uint part) const
	{
		return this->feeder_share * part / static_cast<uint>(this->count);
	}

	/**
	 * Gets the number of days this cargo has been in transit.
	 * This number isn't really in days, but in 2.5 days (CARGO_AGING_TICKS = 185 ticks) and
	 * it is capped at 255.
	 * @return Length this cargo has been in transit.
	 */
	inline byte DaysInTransit() const
	{
		return this->days_in_transit;
	}

	/**
	 * Gets the source of the cargo.
	 * @return Source of the cargo.
	 */
	inline const CargoSource &Source() const
	{
		return this->source;
	}

	/**
	 * Gets the ID of the station where the cargo was loaded for the first time.
	 * @return StationID.
	 */
	inline StationID SourceStation() const
	{
		return this->source_st;
	}

	/**
	 * Gets the coordinates of the cargo's source station.
	 * @return Source station's coordinates.
	 */
	inline TileIndex SourceStationXY() const
	{
		return this->source_xy;
	}

	/**
	 * Gets the coordinates of the cargo's last loading station.
	 * @return Last loading station's coordinates.
	 */
	inline TileIndex LoadedAtXY() const
	{
		return this->loaded_at_xy;
	}

	/**
	 * Gets the ID of station the cargo wants to go next.
	 * @return Next station for this packets.
	 */
	inline StationID NextStation() const
	{
		return this->next_station;
	}

	static void InvalidateAllFrom(SourceType src_type, SourceID src);
	static void InvalidateAllFrom(StationID sid);
	static void AfterLoad(const SavegameTypeVersion *stv);
};

/**
 * Iterate over all _valid_ cargo packets from the given start.
 * @param var   Variable used as "iterator".
 * @param start Cargo packet ID of the first packet to iterate over.
 */
#define FOR_ALL_CARGOPACKETS_FROM(var, start) FOR_ALL_ITEMS_FROM(CargoPacket, cargopacket_index, var, start)

/**
 * Iterate over all _valid_ cargo packets from the begin of the pool.
 * @param var   Variable used as "iterator".
 */
#define FOR_ALL_CARGOPACKETS(var) FOR_ALL_CARGOPACKETS_FROM(var, 0)

/**
 * Simple collection class for a list of cargo packets.
 * @tparam Tinst Actual instantiation of this cargo list.
 */
template <class Tinst, class Tcont>
class CargoList {
public:
	/** The iterator for our container. */
	typedef typename Tcont::iterator Iterator;
	/** The reverse iterator for our container. */
	typedef typename Tcont::reverse_iterator ReverseIterator;
	/** The const iterator for our container. */
	typedef typename Tcont::const_iterator ConstIterator;
	/** The const reverse iterator for our container. */
	typedef typename Tcont::const_reverse_iterator ConstReverseIterator;

	/** Kind of actions that could be done with packets on move. */
	enum MoveToAction {
		MTA_BEGIN = 0,
		MTA_TRANSFER = 0, ///< Transfer the cargo to the station.
		MTA_DELIVER,      ///< Deliver the cargo to some town or industry.
		MTA_KEEP,         ///< Keep the cargo in the vehicle.
		MTA_LOAD,         ///< Load the cargo from the station.
		MTA_END,
		NUM_MOVE_TO_ACTION = MTA_END
	};

protected:
	uint count;                 ///< Cache for the number of cargo entities.
	uint cargo_days_in_transit; ///< Cache for the sum of number of days in transit of each entity; comparable to man-hours.

	Tcont packets;              ///< The cargo packets in this list.

	void AddToCache(const CargoPacket *cp);

	void RemoveFromCache(const CargoPacket *cp, uint count);

public:
	/** Create the cargo list. */
	CargoList() {}

	~CargoList();

	void OnCleanPool();

	/**
	 * Returns a pointer to the cargo packet list (so you can iterate over it etc).
	 * @return Pointer to the packet list.
	 */
	inline const Tcont *Packets() const
	{
		return &this->packets;
	}

	/**
	 * Returns average number of days in transit for a cargo entity.
	 * @return The before mentioned number.
	 */
	inline uint DaysInTransit() const
	{
		return this->count == 0 ? 0 : this->cargo_days_in_transit / this->count;
	}

	void InvalidateCache();
};

typedef std::list<CargoPacket *> CargoPacketList;

/**
 * CargoList that is used for vehicles.
 */
class VehicleCargoList : public CargoList<VehicleCargoList, CargoPacketList> {
protected:
	/** The (direct) parent of this class. */
	typedef CargoList<VehicleCargoList, CargoPacketList> Parent;

	Money feeder_share;                     ///< Cache for the feeder share.
	uint action_counts[NUM_MOVE_TO_ACTION]; ///< Counts of cargo to be transfered, delivered, kept and loaded.

	/**
	 * Assert that the designation counts add up.
	 */
	inline void AssertCountConsistency() const
	{
		assert(this->action_counts[MTA_KEEP] +
				this->action_counts[MTA_DELIVER] +
				this->action_counts[MTA_TRANSFER] +
				this->action_counts[MTA_LOAD] == this->count);
	}

	void AddToCache(const CargoPacket *cp);
	void RemoveFromCache(const CargoPacket *cp, uint count);

	void AddToMeta(const CargoPacket *cp, MoveToAction action);
	void RemoveFromMeta(const CargoPacket *cp, MoveToAction action, uint count);

	static MoveToAction ChooseAction(const CargoPacket *cp, StationID cargo_next,
			StationID current_station, bool accepted,
			const StationIDStack &next_station);

public:
	/** The station cargo list needs to control the unloading. */
	friend class StationCargoList;
	/** The super class ought to know what it's doing. */
	friend class CargoList<VehicleCargoList, CargoPacketList>;
	/** The vehicles have a cargo list (and we want that saved). */
	friend const struct SaveLoad *GetVehicleDescription(VehicleType vt);

	/**
	 * Returns source of the first cargo packet in this list.
	 * @return The before mentioned source.
	 */
	inline StationID Source() const
	{
		return this->count == 0 ? INVALID_STATION : this->packets.front()->SourceStation();
	}

	/**
	 * Returns total sum of the feeder share for all packets.
	 * @return The before mentioned number.
	 */
	inline Money FeederShare() const
	{
		return this->feeder_share;
	}

	/**
	 * Returns the amount of cargo designated for a given purpose.
	 * @param action Action the cargo is designated for.
	 * @return Amount of cargo designated for the given action.
	 */
	inline uint ActionCount(MoveToAction action) const
	{
		return this->action_counts[action];
	}

	/**
	 * Returns sum of cargo on board the vehicle (ie not only
	 * reserved).
	 * @return Cargo on board the vehicle.
	 */
	inline uint StoredCount() const
	{
		return this->count - this->action_counts[MTA_LOAD];
	}

	/**
	 * Returns sum of cargo, including reserved cargo.
	 * @return Sum of cargo.
	 */
	inline uint TotalCount() const
	{
		return this->count;
	}

	/**
	 * Returns sum of reserved cargo.
	 * @return Sum of reserved cargo.
	 */
	inline uint ReservedCount() const
	{
		return this->action_counts[MTA_LOAD];
	}

	/**
	 * Returns sum of cargo to be moved out of the vehicle at the current station.
	 * @return Cargo to be moved.
	 */
	inline uint UnloadCount() const
	{
		return this->action_counts[MTA_TRANSFER] + this->action_counts[MTA_DELIVER];
	}

	/**
	 * Returns the sum of cargo to be kept in the vehicle at the current station.
	 * @return Cargo to be kept or loaded.
	 */
	inline uint RemainingCount() const
	{
		return this->action_counts[MTA_KEEP] + this->action_counts[MTA_LOAD];
	}

	void Append(CargoPacket *cp, MoveToAction action = MTA_KEEP);

	void AgeCargo();

	void InvalidateCache();

	void SetTransferLoadPlace(TileIndex xy);

	bool Stage (bool accepted, StationID current_station,
		const StationIDStack &next_station, uint8 order_flags,
		const GoodsEntry *ge, CargoPayment *payment);

	/**
	 * Marks all cargo in the vehicle as to be kept. This is mostly useful for
	 * loading old savegames. When loading is aborted the reserved cargo has
	 * to be returned first.
	 */
	inline void KeepAll()
	{
		this->action_counts[MTA_DELIVER] = this->action_counts[MTA_TRANSFER] = this->action_counts[MTA_LOAD] = 0;
		this->action_counts[MTA_KEEP] = this->count;
	}

	/**
	 * Marks cargo previously set to load or deliver as to be kept.
	 * @param from Previous designation of cargo (MTA_LOAD or MTA_DELIVER).
	 * @param max_move Maximum amount of cargo to reassign.
	 */
	void Keep (MoveToAction from, uint max_move = UINT_MAX)
	{
		assert (from == MTA_DELIVER || from == MTA_LOAD);
		max_move = min (this->action_counts[from], max_move);
		this->action_counts[from] -= max_move;
		this->action_counts[MTA_KEEP] += max_move;
	}

	/* Methods for moving cargo around. First parameter is always maximum
	 * amount of cargo to be moved. Second parameter is destination (if
	 * applicable), return value is amount of cargo actually moved. */

	void Transfer (void);
	uint Return (StationCargoList *dest, uint max_move = UINT_MAX);
	uint Unload(uint max_move, StationCargoList *dest, CargoPayment *payment);
	uint Shift(uint max_move, VehicleCargoList *dest);
	uint Truncate(uint max_move = UINT_MAX);
	void Reroute (StationID avoid, StationID avoid2, const GoodsEntry *ge);
};

typedef MultiMap<StationID, CargoPacket *> StationCargoPacketMap;
typedef std::map<StationID, uint> StationCargoAmountMap;

/**
 * CargoList that is used for stations.
 */
class StationCargoList : public CargoList<StationCargoList, StationCargoPacketMap> {
protected:
	/** The (direct) parent of this class. */
	typedef CargoList<StationCargoList, StationCargoPacketMap> Parent;

	uint reserved_count; ///< Amount of cargo being reserved for loading.

public:
	/** The super class ought to know what it's doing. */
	friend class CargoList<StationCargoList, StationCargoPacketMap>;
	/** The stations, via GoodsEntry, have a CargoList. */
	friend const struct SaveLoad *GetGoodsDesc();

	friend class VehicleCargoList;

	friend class CargoLoad;
	friend class CargoReservation;

	template<class Taction>
	uint ShiftCargo (Taction action, const StationIDStack &next);

	void Append(CargoPacket *cp, StationID next);

	/**
	 * Check for cargo headed for a specific station.
	 * @param next Station the cargo is headed for.
	 * @return If there is any cargo for that station.
	 */
	inline bool HasCargoFor (const StationIDStack &next) const
	{
		for (StationIDStack::const_iterator iter (next.begin()); iter != next.end(); iter++) {
			if (this->packets.find (*iter) != this->packets.end()) return true;
		}
		/* Packets for INVALID_STTION can go anywhere. */
		return this->packets.find(INVALID_STATION) != this->packets.end();
	}

	/**
	 * Returns source of the first cargo packet in this list.
	 * @return The before mentioned source.
	 */
	inline StationID Source() const
	{
		return this->count == 0 ? INVALID_STATION : this->packets.begin()->second.front()->SourceStation();
	}

	/**
	 * Returns sum of cargo still available for loading at the sation.
	 * (i.e. not counting cargo which is already reserved for loading)
	 * @return Cargo on board the vehicle.
	 */
	inline uint AvailableCount() const
	{
		return this->count;
	}

	/**
	 * Returns sum of cargo reserved for loading onto vehicles.
	 * @return Cargo reserved for loading.
	 */
	inline uint ReservedCount() const
	{
		return this->reserved_count;
	}

	/**
	 * Returns total count of cargo at the station, including
	 * cargo which is already reserved for loading.
	 * @return Total cargo count.
	 */
	inline uint TotalCount() const
	{
		return this->count + this->reserved_count;
	}

	/* Methods for moving cargo around. First parameter is always maximum
	 * amount of cargo to be moved. Second parameter is destination (if
	 * applicable), return value is amount of cargo actually moved. */

	uint Reserve (uint max_move, VehicleCargoList *dest, TileIndex load_place, const StationIDStack &next);
	uint Load (uint max_move, VehicleCargoList *dest, TileIndex load_place, const StationIDStack &next);
	uint Truncate(uint max_move = UINT_MAX, StationCargoAmountMap *cargo_per_source = NULL);
	void Reroute (StationID avoid, StationID avoid2, const GoodsEntry *ge);
};

#endif /* CARGOPACKET_H */
