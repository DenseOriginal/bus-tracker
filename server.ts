import { departuresEndpoint, fetchMidttrafikAPI } from "./src/api.ts";
import { Stops } from "./src/constants/stops.ts";
import { WalkingTimes } from "./src/constants/walking_time.ts";
import { Departure } from "./src/types.ts";
import { getDepartureDiffInMinutes, getDepartureTime } from "./src/utils.ts";
import { loadSync } from "@std/dotenv";
import dayjs from 'dayjs';
import utc from 'dayjs/plugin/utc.js';
import timezone from 'dayjs/plugin/timezone.js';

dayjs.extend(utc);
dayjs.extend(timezone);

loadSync({ export: true });

async function getSoonestDepartures() {
	const kantorvængetDepartures = await fetchMidttrafikAPI(departuresEndpoint(Stops.KantorvængetModKolt));
	const vejlbyDepartures = await fetchMidttrafikAPI(departuresEndpoint(Stops.VejlbyModKolt));

	const possibleKantorvængetDeparture = kantorvængetDepartures
		.departures
		.filter(departure => departure.name == "1A" && !departure.cancelled)
		.filter(departure => getDepartureDiffInMinutes(departure) > WalkingTimes.ToKantorvænget)
		.sort((a, b) => getDepartureDiffInMinutes(a) - getDepartureDiffInMinutes(b));

	const possibleVejlbyDeparture = vejlbyDepartures.departures
		.filter(departure => getDepartureDiffInMinutes(departure) > WalkingTimes.ToVejlby)
		.sort((a, b) => getDepartureDiffInMinutes(a) - getDepartureDiffInMinutes(b));


	const soonestKantorvænget = possibleKantorvængetDeparture[0];
	const soonestVejlby = possibleVejlbyDeparture[0];

	const mapper = (dep: Departure, walkTime: number) => ({
			name: dep.stop.replace(" (Aarhus Kom)", ""),
			leaveIn: getDepartureDiffInMinutes(dep) - walkTime,
			datetime: getDepartureTime(dep)
		})

	return {
		vejlby: soonestVejlby ? mapper(soonestVejlby, WalkingTimes.ToVejlby) : null,
		skejby: soonestKantorvænget ? mapper(soonestKantorvænget, WalkingTimes.ToKantorvænget) : null,
		denmarkNow: dayjs().tz('Europe/Copenhagen').format(),

	};
}

Deno.serve({ port: 4242, hostname: Deno.env.get("HOSTNAME") }, async () => {
	const departures = await getSoonestDepartures();
	console.log(`[${new Date().toISOString()}]`, JSON.stringify(departures));
	
	
	return new Response(JSON.stringify(departures), {
		headers: { "Content-Type": "application/json" },
	});
})