type Brand<B, T> = { readonly __brand: B, readonly __type: T };
export type Nominal<T, B> = T & Brand<B, T>;
export type UnwrapNominal<T> = T extends Brand<infer U, infer _> ? _ : never;

export type StopID = Nominal<string, "StopID">;
export type BusID = Nominal<string, "BusID">;
export type JourneyID = Nominal<string, "JourneyID">;
export type DepartureRef = Nominal<string, "DepartureRef">;

export interface Stop {
	/** Stop ID */
	number: StopID;
	name: string;
	latitude: number;
	longitude: number;
	types: Record<'empty', never> | { 2: "2" };
	distance: number;
}

export interface Departure {
	name: string;
	stop: string;
	dateTime: string;
	rtDateTime: string | null;
	realRtDateTime: string | null;
	direction: string;
	cancelled: boolean;
	ref: DepartureRef;
	finalStop: string;
	customerTour: string;
	journeyNotStarted: boolean;
	activeBus: ActiveBus | null;
}

export interface ActiveBus {
	departureDateTime: string;
	endStation: string;
	customerTour: string;
	journeyId: JourneyID;
	id: BusID;
	passengerCountInformation: PassengerCountInformation;
}

export interface JourneyStop {
	/** Stop ID */
	number: StopID;
	name: string;
	stopIndex: number;
	stopPassed: boolean;
	latitude: number;
	longitude: number;
	arriveTime: string;
	departureTime: string;
	rtArriveTime: string | null;
	rtDepartureTime: string | null;
	cancelled: boolean;
}

export interface Bus {
	id: BusID;
	line: string;
	latitude: number;
	longitude: number;
	directionText: string;
	journeyId: JourneyID;
	endStation: string;
	trafficTypeCode: import("./constants/trafic_types.ts").TraficType;
	customerTour: string;
	colors: {
		colorForeground: string;
		colorBackground: string;
		colorText: string;
	};
	passengerCountInformation: PassengerCountInformation;
}

export interface PassengerCountInformation {
	seats: {
		0: { color: string };
		1: { color: string };
		2: { color: string };
		3: { color: string };
		4: { color: string };
	};
	accessibilityLabel: string;
	capacityDetails: {
		passengerSeats: {
			0: PassengerSeat;
			1: PassengerSeat;
			2: PassengerSeat;
			3: PassengerSeat;
			4: PassengerSeat;
		};
		accessibilityLabel: string;
	};
	passengerCountAllowed: number;
	passengerCountCurrent: number;
	passengerCountTransferred: number;
}

export type PassengerSeat = { color: string; iconName: string; isOccupied: boolean; isDisabled: boolean };

// API Reponse Interfaces

// Bus journey stop
// api/buses/stops/e208cbf8-a30b-4137-a00d-e42f969c4ee8
export interface JourneyStopResponse {
	stops: JourneyStop[];
	passengerCountInformation: PassengerCountInformation;
	tssAnnouncements: unknown[];
	journeyChange: unknown | null;
	routeCoordinates: unknown[];
}

// Stop departures
// api/stops/departures/751417401
export interface DeparturesResponse {
	departures: Departure[];
}

// Get active busses
// api/buses/routeMultiple?lineEnd[0]=1A|751462702&lineEnd[1]=1A|751422004&lineEnd[2]=1A|751418202
// api/buses?latitude=56.160255835&longitude=10.180006&radius=18342.18946307024
export interface ActiveBussesResponse {
	busses: Bus[];
}

// Get bus
// api/buses/1442
export interface BusResponse {
	bus: Bus;
}

export interface MidttrafikResponse<Data> {
	data: Data;
	status: number;
	errors: unknown | null;
}