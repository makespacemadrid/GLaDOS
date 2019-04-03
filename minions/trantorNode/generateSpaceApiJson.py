def generateSpaceApi(status,coffeMade) :

	import time
	lastChange   = int(time.time())
	imgPaths     = "http://makespacemadrid.org/spaceapi"
	spaceOpen    = "" 
	spaceMessage = ""
	if status == True :
		spaceOpen    = True 
		spaceMessage = "Making"
	else:
		spaceOpen    = False 
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
				[{"name"  : "Coffe made",
				 "value" : coffeMade}]
		},		
		"location": {
			"lat": 40.399525, 
			"lon": -3.702446, 
			"address": "Calle Arquitectura 18 , 28045 Madrid, Spain"
		}, 
	}

	import json
	with open('status.json', 'w') as outfile:
		json.dump(data, outfile)

#generateSpaceApi(False,100)
