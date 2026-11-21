import logging
from urllib.parse import quote_plus
from pymongo.mongo_client import MongoClient
from pymongo.server_api import ServerApi

logger = logging.getLogger(__name__)


class MongoConnection:
    def __init__(
        self,
        username: str = "",
        password: str = "",
        cluster_uri: str = "",
        databaseName: str = "HealthCare",
        collectionName: str = "profile data",
    ) -> None:
        if cluster_uri:
            uri = cluster_uri
        elif username and password:
            user_enc = quote_plus(username)
            pass_enc = quote_plus(password)
            uri = f"mongodb+srv://{user_enc}:{pass_enc}@healthcare.emzypae.mongodb.net/?retryWrites=true&w=majority&appName=HealthCare"
        else:
            raise ValueError("MongoDB credentials missing: set MONGO_CLUSTER_URI or MONGO_USERNAME/MONGO_PASSWORD.")

        client = MongoClient(uri, server_api=ServerApi("1"), serverSelectionTimeoutMS=5000)
        myDb = client[databaseName]
        self.myCol = myDb[collectionName]

    def createData(self, data: dict):
        return self.myCol.insert_one(data)

    def initDevice(self, deviceid: int):
        res = self.myCol.find_one({"device": deviceid})
        if res:
            return res
        self.myCol.insert_one({"device": deviceid, "client": 0})
        return {"device": deviceid, "client": 0}

    def assignDevice(self, deviceid: int, doctorid: int, clientid: int):
        self.myCol.update_many(
            {f"devices.{deviceid}": {"$exists": True}},
            {"$unset": {f"devices.{deviceid}": ""}},
        )
        return self.myCol.find_one_and_update(
            {"_id": doctorid},
            {"$set": {f"devices.{deviceid}": clientid}},
            return_document=True,
        )

    def assignUser(self, doctorid: int, clientid: int, deviceid: int):
        filter_doc = {"_id": doctorid}
        try:
            res = self.myCol.find_one(filter_doc)
            if res and len(res.get("devices", {})) > 1:
                for key, val in res["devices"].items():
                    if val == clientid:
                        self.myCol.find_one_and_update(filter_doc, {"$set": {f"devices.{key}": 0}})
                        break
        except Exception as exc:
            logger.warning("Error clearing prior device mapping: %s", exc)

        return self.myCol.find_one_and_update(
            filter_doc,
            {"$set": {f"devices.{deviceid}": clientid}},
            return_document=True,
        )

    def assignConnection(self, deviceid: int, clientid: int):
        try:
            self.myCol.find_one_and_update({"client": clientid}, {"$set": {"client": 0}})
        except Exception as exc:
            logger.debug("No prior device assigned to client %s: %s", clientid, exc)

        return self.myCol.find_one_and_update(
            {"device": deviceid},
            {"$set": {"client": clientid}},
            return_document=True,
        )

    def find(self, _id: int):
        res = self.myCol.find_one({"_id": _id})
        if res and "_id" in res:
            res["_id"] = int(res["_id"])
        return res

    def findUserId(self, email: str, password: str | None = None):
        query = {"email": email, "password": password} if password else {"email": email}
        res = self.myCol.find_one(query)
        return res["_id"] if res else None

    def getClientID(self, deviceid: int):
        res = self.myCol.find_one({"device": deviceid})
        return res["client"] if res else None
