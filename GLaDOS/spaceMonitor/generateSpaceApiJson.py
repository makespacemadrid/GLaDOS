def generateSpaceApi(status,coffeMade) :

	import time
	lastChange   = int(time.time())
	imgPaths     = "http://makespacemadrid.sytes.net/spaceapi"
	spaceOpen    = "" 
	spaceMessage = ""
	if status == True :
		spaceOpen    = "true" 
		spaceMessage = "Making"
	else:
		spaceOpen    = "false" 
		spaceMessage = "Off"

	data = {
		"api": "0.13", 
		"space": "MakeSpace Madrid", 
		"url": "http://makespacemadrid.org/", 
		"logo": imgPaths+"/logo.png",
		"state": {
		"lastchange": lastChange, 
		"message": spaceMessage, 
		"open": spaceOpen, 
		"icon": {
				"open": imgPaths+"/icon-open.png", 
				"closed": imgPaths+"/icon-closed.png"
			},
		}, 
		"contact": {
		"ml": "info@makespacemadrid.org", 
		"twitter": "@MakeSpaceMadrid", 
		}, 
		"sensors": {
			"coffe_machine0":
				{
					"coffe_made": coffeMade
				}
		},		
		"location": {
			"lat": 40.403600, 
			"lon": -3.694596, 
			"address": "Pedro Unanue 16 , 28045 Madrid, Spain"
		}, 
	}

	import json
	with open('status.json', 'w') as outfile:
		json.dump(data, outfile)
