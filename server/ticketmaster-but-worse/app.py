import secrets, base64
import pyotp
from flask import Flask, json, jsonify, request
from flask_jwt_extended import JWTManager, create_access_token, current_user, jwt_required

app = Flask(__name__)

app.config["JWT_SECRET_KEY"] = secrets.token_urlsafe(64)
jwt = JWTManager(app)

#these dicts simulate the servers database
tickets = {} #key: ticket token, value: base32 encoded totp_secret
users = {
    "defaultUser": {
        "username": "defaultUser",
        "password": "password1234",
        "tickets": [], #stores ticket tokens which then can be used to lookup the totp_secret in the tickets dict above
    }
}

def generate_ticket(username):
    """
    This function creates a new ticket for the specified user.
    In reality this would be executed every time a user buys a new ticket, 
    for the sake of this demo we just run it ones for 'defaultUser' at startup
    """
    token = secrets.token_urlsafe()
    totp_secret = pyotp.random_base32()

    tickets[token] = totp_secret 
    users[username]["tickets"].append(token)

#our default user gets one ticket
generate_ticket("defaultUser")

@jwt.user_lookup_loader
def user_lookup_loader(_, jwt_data):
    username = jwt_data["sub"]
    return users[username]

#test route used by bash script to check if server is online
@app.get("/ping")
def ping():
    return "pong"

@app.post("/login")
def login():
    username = request.form["username"]
    password = request.form["password"]
    user = users.get(username)
    if user and password == user["password"]:
        access_token = create_access_token(identity=username)
        return jsonify(client_token=access_token)
    return jsonify(msg="Login invalid"), 403

@app.get("/get_tickets")
@jwt_required()
def get_tickets():
    user = current_user
    returnVal = []
    for token in users[user["username"]]["tickets"]:
        returnVal.append({
            "token": token,
            "totp_secret": tickets[token]
        })
    return jsonify(tickets=returnVal)

@app.get("/check_ticket_validity")
def validate_ticket():
    """
    In reality this API endpoint would only be available to the devices used by the staff at events that scan and validate the tickets.
    We use it in the client application to make sure that everything works and to point people to it so that they can easily make sure that they solved the challenge (without requiring them to generate barcodes as well)
    Note that jwt_required is missing here, no authentication by a user account is needed.
    This is the first major flaw in this code: This function doesn't check which account owns the ticket, it just checks if the ticket exists and is valid at all. This would also be quite difficult to do with just a barcode.
    """
    base32Object = request.args["ticket"]
    jsonObjectAsString = base64.b32decode(base32Object).decode('ascii')
    jsonObject = json.loads(jsonObjectAsString)

    #lookup ticket using the hash of its token
    ticket_secret = tickets.get(jsonObject['token'])
    if not ticket_secret:
        return jsonify(msg=f"No such ticket exists"), 404

    totp = pyotp.TOTP(ticket_secret, digits=10, interval=15)
    if totp.verify(jsonObject['one_time_password']):
        return jsonify(msg=f"Success, this ticket is still valid")
    else:
        return jsonify(msg=f"This ticket is invalid!"), 403
