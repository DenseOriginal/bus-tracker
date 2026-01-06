import { Departure } from "./types.ts";

export const getDepartureTime = (departure: Departure): string => {
	return departure.realRtDateTime || departure.rtDateTime || departure.dateTime;
}

export const getDiffInMinutes = (datetime: string): number => {
	const denmarkTime = new Date(new Date(datetime).toLocaleString('en-US', { timeZone: 'Europe/Copenhagen' }));
	return Math.floor((denmarkTime.getTime() - Date.now()) / 60000);
}

export const getDepartureDiffInMinutes = (departure: Departure): number => getDiffInMinutes(getDepartureTime(departure));

export const logDeparture = (departure: Departure): void => {
	console.log(
		departure.stop.padEnd(40, " "),
		"leaving in",
		getDepartureDiffInMinutes(departure),
		"minutes"
	)
}