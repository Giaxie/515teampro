// src/firebase.js
import { initializeApp } from "firebase/app";
import { getDatabase, ref, onValue } from "firebase/database";

const firebaseConfig = {
  apiKey: "AIzaSyAT_C9tZRRwqm2oBCOnAOHl17FeQkvPlUw",
  authDomain: "co-play-a9a40.firebaseapp.com",
  databaseURL: "https://co-play-a9a40-default-rtdb.firebaseio.com",
  projectId: "co-play-a9a40",
  storageBucket: "co-play-a9a40.appspot.com",
};

const app = initializeApp(firebaseConfig);
const db = getDatabase(app);

export { db, ref, onValue };