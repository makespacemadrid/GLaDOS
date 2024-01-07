"""
    The purpose of this web application is to setup the access control lists
    of a network of RFID stations in the MakerSpace.
"""

from hashlib import sha256

from flask import Flask, render_template, request
from flask_httpauth import HTTPBasicAuth

# Configuration
PORT = 5000

class RfidIdentity():
    """
        RFID identity and its associated user.
    """
    def __init__(self, rfid_id):
        self.id = rfid_id

class RfidStation:
    """
        RFID device equiped with an RFID reader.
    """
    def __init__(self, station_id: str, allowed_rfids_list: list[RfidIdentity]):
        self.station_id = station_id
        self.allowed_rfids_list = allowed_rfids_list



class RfidDb:
    """
        Database adapter for the following tasks:
            - Submit a new RFID to the database
            - Check if an ID is allowed in a station
            - Add ID to the station
            - Remove ID from the station
    """
    def __generate_password_hash(self, user, password):
        """
            Get the hash for the user and the password.
        """
        return sha256((user+password).encode("utf-8")).hexdigest()

    def __init__(self):
        # Testing data until we have a data layer figured out.
        laser_station = RfidStation("laser", [3, 4])
        puerta_station = RfidStation("puerta", [1, 2, 3, 4])
        reactor_station = RfidStation("reactor", [1, 3])

        self.stations = {
            "laser" : laser_station,
            "puerta" : puerta_station,
            "reactor": reactor_station,
        }

        self.users = {
            "test" : self.__generate_password_hash("test", "1234")
        }

    def get_station_acl(self, station_id) -> list:
        """
            Return the list of allowed rfids for a given station.

            This is the API endpoint that should be retrieved by the RFID station.
        """
        return self.stations[station_id].allowed_rfids_list

    def check_rfid_is_allowed_in_station(self, station_id, rfid_number) -> (bool, str):
        """ Return True if the rfid read on this station is allowed and False otherwise. """
        if station_id not in self.stations:
            return False, "Undefined station"
        acl = self.get_station_acl(station_id)
        return rfid_number in acl, None

    def add_rfid_to_station(self, station_id, rfid_number) -> (bool, str):
        """
            Adds rfid to station if this one exists and is not already on the ACL.
        """
        if station_id not in self.stations:
            return False, "Undefined station"
        acl = self.get_station_acl(station_id)
        if rfid_number in acl:
            return False, "rfid number already in station ACL"
        self.stations[station_id].allowed_rfids_list.append(rfid_number)
        print(f"=== New rfid '{rfid_number}' added to station '{station_id}'")
        return True, None


    def check_user(self, user, password) -> bool:
        """
            Returns true if the given user and password matches with the one in the database.
        """
        if user not in self.users:
            return False
        if self.users[user] != self.__generate_password_hash(user, password):
            return False
        return True

# Application Singlentons
app = Flask(__name__)
db = RfidDb()
auth = HTTPBasicAuth()

@auth.verify_password
def verify_password(username, password):
    """
        Check the username and password from the database.
    """
    return db.check_user(username, password)

@app.route("/", methods=["GET", "POST"])
@auth.login_required
def home():
    "Root level webpage"

    if request.method == "POST":
        success, err = db.add_rfid_to_station(request.form["station_id"], request.form["new_rfid"])
        if not success:
            print(f"[ERROR] Could not add new rfid to station: {err}")
            return render_template("error-501.html")
        return render_template("submited.html")

    return render_template("add-rfid-to-station.html")

def main():
    "Main flow of execution"
    app.run(host="0.0.0.0", debug=True, port=PORT)

if __name__ == "__main__":
    main()
