import { Departure } from "./types.ts";
import dayjs from 'dayjs';
import utc from 'dayjs/plugin/utc.js';
import timezone from 'dayjs/plugin/timezone.js';

export const getDepartureTime = (departure: Departure): string => {
	return departure.realRtDateTime || departure.rtDateTime || departure.dateTime;
}

dayjs.extend(utc);
dayjs.extend(timezone);

export const getDiffInMinutes = (datetime: string): number => {
	const denmarkNow = dayjs().tz('Europe/Copenhagen');
	const departureTime = dayjs.tz(datetime, 'Europe/Copenhagen');
	return Math.floor(departureTime.diff(denmarkNow, 'minute', true));
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