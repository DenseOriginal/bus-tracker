import {
	ActiveBussesResponse,
	BusID,
	BusResponse,
	DeparturesResponse,
	JourneyID,
	JourneyStopResponse,
	MidttrafikResponse,
	StopID
} from "./types.ts";
import { loadSync } from "@std/dotenv";

type MidttrafikEndpoint<ResponseData> = string & { __midttrafikEndpointResponse: MidttrafikResponse<ResponseData> };
type GetResponseType<E> = E extends MidttrafikEndpoint<infer R> ? R : never;

loadSync({ export: true });

const baseUrl = Deno.env.get("MIDTTRAFIK_API_BASE_URL");
if (!baseUrl) throw new Error("MIDTTRAFIK_API_BASE_URL is not defined in environment variables");

const e = <ResponseData>(path: string) => baseUrl + path as MidttrafikEndpoint<ResponseData>;

const busJourneyStopsEndpoint = (journeyId: JourneyID) => e<JourneyStopResponse>(`buses/stops/${journeyId}`);
const departuresEndpoint = (stopId: StopID) => e<DeparturesResponse>(`stops/departures/${stopId}`);
const getActiveBussesEndpoint = (lat: number, lon: number, radiusMeters: number) =>
	e<ActiveBussesResponse>(`buses?latitude=${lat}&longitude=${lon}&radius=${radiusMeters}`);

const getAllActiveBussesEndpoint = () => getActiveBussesEndpoint(56.160255835, 10.180006, 18342.18946);
const getBussesByRouteEndpoint = (lines: { name: string; finalStopId: StopID }[]) => {
	const queryParams = lines
		.map((line, index) => `lineEnd[${index}]=${line.name}|${line.finalStopId}`)
		.join("&");
	return e<ActiveBussesResponse>(`buses/routeMultiple?${queryParams}`);
}

const getBusEndpoint = (busId: BusID) => e<BusResponse>(`buses/${busId}`);

async function fetchMidttrafikAPI<E extends MidttrafikEndpoint<unknown>>(endpoint: E): Promise<GetResponseType<E>> {
	const response = await fetch(endpoint)
	if (!response.ok) {
		console.error(await response.text());
		throw new Error(`Midttrafik API request failed with status ${response.status}: ${response.statusText}`);
	}

	const parsed: MidttrafikResponse<GetResponseType<E>> = await response.json()
	return parsed.data;
}

export {
	busJourneyStopsEndpoint,
	departuresEndpoint,
	getActiveBussesEndpoint,
	getAllActiveBussesEndpoint,
	getBussesByRouteEndpoint,
	getBusEndpoint,
	fetchMidttrafikAPI,
};