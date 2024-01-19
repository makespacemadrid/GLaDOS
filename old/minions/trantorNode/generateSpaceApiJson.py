def generateSpaceApi(status,temperature,humidity,coffeMade) :

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
		"email": "info@makespacemadrid.org", 
		"twitter": "@MakeSpaceMadrid", 
		}, 
		"issue_report_channels": ["email"],
		"sensors": {
			"space": [{"name"  : "Temperature", "value" : temperature},
				{"name"  : "Humidity", "value" : humidity},
				{"name"  : "Coffe made", "value" : coffeMade}]
		},		
		"location": {
			"lat": 40.399525, 
			"lon": -3.702446, 
			"address": "Calle Arquitectura 18 , 28005 Madrid, Spain"
		},
		"projects": [
		"https://github.com/makesapacemadrid",
		"https://docs.makespacemadrid.org/doku.php?id=Proyectos"
  ] 
	}

	import json
	with open('status.json', 'w') as outfile:
		json.dump(data, outfile)

#generateSpaceApi(False,26,20,100)
