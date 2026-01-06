import { BusID, DepartureRef, UnwrapNominal, JourneyID, Nominal, StopID } from "../types.ts";

export const toNominal = <T extends Nominal<unknown, string>>() => (data: UnwrapNominal<T>): T => {
	return data as T;
}

export const toStopID = toNominal<StopID>();
export const toBusID = toNominal<BusID>();
export const toJourneyID = toNominal<JourneyID>();
export const toDepartureRef = toNominal<DepartureRef>();