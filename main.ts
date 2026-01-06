import { departuresEndpoint, fetchMidttrafikAPI } from "./src/api.ts";
import { Stops } from "./src/constants/stops.ts";
import { WalkingTimes } from "./src/constants/walking_time.ts";
import { getDepartureDiffInMinutes, getDepartureTime } from "./src/utils.ts";

const kantorvængetDepartures = await fetchMidttrafikAPI(departuresEndpoint(Stops.KantorvængetModKolt));
const vejlbyDepartures = await fetchMidttrafikAPI(departuresEndpoint(Stops.VejlbyModKolt));

const possibleKantorvængetDeparture = kantorvængetDepartures
	.departures
	.filter(departure => departure.name == "1A" && !departure.cancelled)
	.filter(departure => getDepartureDiffInMinutes(departure) > WalkingTimes.ToKantorvænget);

const possibleVejlbyDeparture = vejlbyDepartures.departures
	.filter(departure => getDepartureDiffInMinutes(departure) > WalkingTimes.ToVejlby);

const allDepartures = [
	...possibleKantorvængetDeparture,
	...possibleVejlbyDeparture
].sort((a, b) => getDepartureDiffInMinutes(a) - getDepartureDiffInMinutes(b))
	.slice(0, 5)
	.map(dep => ({
		name: dep.stop.replace(" (Aarhus Kom)", ""),
		arrivingIn: getDepartureDiffInMinutes(dep),
		datetime: getDepartureTime(dep)
	}));

console.log(allDepartures);
