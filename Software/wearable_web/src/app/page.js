"use client";

import { useState, useEffect } from "react";
import dynamic from "next/dynamic";
import "leaflet/dist/leaflet.css";
import mockData from "../data/mockData";

// Dynamically load map components (client-side only)
const MapContainer = dynamic(
  () => import("react-leaflet").then((mod) => mod.MapContainer),
  { ssr: false }
);
const TileLayer = dynamic(
  () => import("react-leaflet").then((mod) => mod.TileLayer),
  { ssr: false }
);
const Marker = dynamic(
  () => import("react-leaflet").then((mod) => mod.Marker),
  { ssr: false }
);
const Popup = dynamic(
  () => import("react-leaflet").then((mod) => mod.Popup),
  { ssr: false }
);

export default function Home() {
  const [data, setData] = useState(mockData);
  const [arrowIcon, setArrowIcon] = useState(null);

  useEffect(() => {
    // TODO: Khi có Firebase thì khởi tạo và gọi setData(firebaseData) ở đây

    // Khởi tạo custom arrow icon chỉ khi chạy trên client
    import("leaflet").then((L) => {
      const icon = L.divIcon({
        html: `<div style="
            transform: rotate(0deg);
            font-size: 32px;
            color: #2563eb;
            text-shadow: 0 0 4px rgba(0,0,0,0.3);
          ">▲</div>`,
        iconSize: [32, 32],
        iconAnchor: [16, 16],
        className: "",
      });
      setArrowIcon(icon);
    });
  }, []);

  // Lấy entry mới nhất từ object timestamp-keyed
  const getLatest = (obj) => {
    if (!obj) return null;
    const keys = Object.keys(obj);
    if (!keys.length) return null;
    const lastKey = keys.sort().pop();
    return { ...obj[lastKey], timestamp: lastKey };
  };

  const hr = getLatest(data.heart_rate)?.bpm;
  const spo2 = getLatest(data.spo2)?.percentage;
  const calHR = getLatest(data.calories_by_heart_rate)?.calories_burned_hr;
  const calAcc = getLatest(data.calories_by_accelerometer)?.total_calories;
  const dailyCal = getLatest(data.calories_daily)?.combined_daily_calories;
  const sleepDur = getLatest(data.daily_sleep)?.sleep_duration;
  const lastSleep = getLatest(data.sleep);
  const loc = data.latest_location;

  return (
    <div className="flex h-screen bg-gray-100">
      {/* Left pane: metrics cards */}
      <div className="w-1/2 overflow-y-auto p-8 grid gap-6 bg-gray-100">
        <Card title="Heart Rate" value={hr ? `${hr} bpm` : "–"} />
        <Card title="SpO₂" value={spo2 ? `${spo2}%` : "–"} />
        <Card
          title="Calories (HR)"
          value={calHR ? `${calHR} kcal` : "–"}
        />
        <Card
          title="Calories (Accel.)"
          value={calAcc ? `${calAcc} kcal` : "–"}
        />
        <Card
          title="Total Calories Today"
          value={dailyCal ? `${dailyCal} kcal` : "–"}
        />
        <Card
          title="Sleep Duration Today"
          value={sleepDur ? `${sleepDur} hrs` : "–"}
        />
        {lastSleep && (
          <Card
            title="Last Sleep Session"
            value={`${lastSleep.duration_hours} hrs (${new Date(
              lastSleep.start_time
            ).toLocaleTimeString()} - ${new Date(
              lastSleep.end_time
            ).toLocaleTimeString()})`}
          />
        )}
      </div>

      {/* Right pane: map */}
      <div className="w-1/2">
        {loc && arrowIcon && (
          <MapContainer
            center={[loc.latitude, loc.longitude]}
            zoom={15}
            style={{ height: "100%", width: "100%" }}
          >
            <TileLayer url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png" />
            <Marker position={[loc.latitude, loc.longitude]} icon={arrowIcon}>
              <Popup>
                Vị trí hiện tại<br />
                Lat: {loc.latitude.toFixed(5)}<br />
                Lng: {loc.longitude.toFixed(5)}
              </Popup>
            </Marker>
          </MapContainer>
        )}
      </div>
    </div>
  );
}

function Card({ title, value }) {
  return (
    <div className="bg-white rounded-2xl border border-gray-200 shadow-md p-6 hover:shadow-lg transition-shadow">
      <h3 className="text-lg font-semibold text-gray-800 mb-2">{title}</h3>
      <p className="text-3xl font-bold text-gray-900">{value}</p>
    </div>
  );
}
