1A mod Kolt/Kolt Kirkevej: 1A|751422004
1A mod Skejbyparken: 1A|751418202
1A mod Vejlby/Stenagervej: 1A|751462702

Get all stops:
<MIDTTRAFIK_API_BASE_URL>/stops?latitude=56.160255835&longitude=10.180006&radius=18342.18946307024

Get all 1A busses:
<MIDTTRAFIK_API_BASE_URL>/buses/routeMultiple?lineEnd[0]=1A|751462702&lineEnd[1]=1A|751422004&lineEnd[2]=1A|751418202

Get departures
<MIDTTRAFIK_API_BASE_URL>/stops/departures/751462702
<MIDTTRAFIK_API_BASE_URL>/stops/departures/751462702?overrideDateTime=2026-01-05T22:15:00

Get journey details
<MIDTTRAFIK_API_BASE_URL>/stops/journeydetails BODY: {journeyRef: string}

Get get journey stops (Realtime)
<MIDTTRAFIK_API_BASE_URL>/buses/stops/72e12d21-dcbf-4317-9796-708ac977663e