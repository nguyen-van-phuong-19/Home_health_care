import React, { useEffect, useState } from 'react';
import { collection, query, where, orderBy, limit, onSnapshot } from 'firebase/firestore';
import { db } from '../firebase';
import { MapContainer, TileLayer, Marker, Popup } from 'react-leaflet';
import 'leaflet/dist/leaflet.css';
import './Location.css';

// Fix for Leaflet marker icon issue in React
import L from 'leaflet';
delete L.Icon.Default.prototype._getIconUrl;
L.Icon.Default.mergeOptions({
  iconRetinaUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/1.7.1/images/marker-icon-2x.png',
  iconUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/1.7.1/images/marker-icon.png',
  shadowUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/1.7.1/images/marker-shadow.png',
});

const Location = ({ userId }) => {
  const [locationData, setLocationData] = useState({
    current: { latitude: 0, longitude: 0, altitude: 0, timestamp: '' },
    history: []
  });
  
  useEffect(() => {
    // Get current location
    const userDocRef = db.collection('users').doc(userId);
    
    const unsubscribe1 = onSnapshot(userDocRef, (docSnap) => {
      if (docSnap.exists()) {
        const data = docSnap.data();
        if (data.latest_location) {
          setLocationData(prev => ({
            ...prev,
            current: data.latest_location
          }));
        }
      }
    });
    
    // Get location history for the last 24 hours
    const now = new Date();
    const yesterday = new Date(now.getTime() - (24 * 60 * 60 * 1000));
    
    const locationCollectionRef = collection(db, 'users', userId, 'gps');
    const locationQuery = query(
      locationCollectionRef,
      where('timestamp', '>=', yesterday.toISOString()),
      orderBy('timestamp', 'asc')
    );
    
    const unsubscribe2 = onSnapshot(locationQuery, (querySnapshot) => {
      const locationHistory = [];
      
      querySnapshot.forEach((doc) => {
        const data = doc.data();
        locationHistory.push({
          latitude: data.latitude,
          longitude: data.longitude,
          altitude: data.altitude,
          timestamp: data.timestamp
        });
      });
      
      setLocationData(prev => ({
        ...prev,
        history: locationHistory
      }));
    });
    
    return () => {
      unsubscribe1();
      unsubscribe2();
    };
  }, [userId]);
  
  const formatTimestamp = (timestamp) => {
    if (!timestamp) return 'N/A';
    return new Date(timestamp).toLocaleString();
  };
  
  // Center map on current location or default to a fallback position
  const mapCenter = locationData.current.latitude && locationData.current.longitude
    ? [locationData.current.latitude, locationData.current.longitude]
    : [51.505, -0.09]; // Default to London if no location available
  
  return (
    <div className="location-page">
      <h1>Location Tracking</h1>
      
      <div className="current-location-details">
        <h2>Current Location</h2>
        <div className="location-coordinates">
          <div className="coordinate-box">
            <h3>Latitude</h3>
            <p>{locationData.current.latitude.toFixed(6)}°</p>
          </div>
          <div className="coordinate-box">
            <h3>Longitude</h3>
            <p>{locationData.current.longitude.toFixed(6)}°</p>
          </div>
          <div className="coordinate-box">
            <h3>Altitude</h3>
            <p>{locationData.current.altitude.toFixed(1)} m</p>
          </div>
        </div>
        <p className="timestamp">Last updated: {formatTimestamp(locationData.current.timestamp)}</p>
      </div>
      
      <div className="map-container">
        <MapContainer center={mapCenter} zoom={13} style={{ height: '400px', width: '100%' }}>
          <TileLayer
            url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
            attribution='&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
          />
          
          {/* Current location marker */}
          <Marker position={mapCenter}>
            <Popup>
              Current Location<br />
              {formatTimestamp(locationData.current.timestamp)}
            </Popup>
          </Marker>
          
          {/* History markers */}
          {locationData.history.map((location, index) => (
            <Marker 
              key={index} 
              position={[location.latitude, location.longitude]}
              opacity={0.7}
            >
              <Popup>
                Previous Location<br />
                {formatTimestamp(location.timestamp)}
              </Popup>
            </Marker>
          ))}
        </MapContainer>
      </div>
      
      <div className="location-history">
        <h2>Location History</h2>
        <div className="history-list">
          {locationData.history.length > 0 ? (
            locationData.history.map((location, index) => (
              <div key={index} className="history-item">
                <p>
                  <strong>Time:</strong> {formatTimestamp(location.timestamp)}<br />
                  <strong>Coordinates:</strong> {location.latitude.toFixed(6)}°, {location.longitude.toFixed(6)}°<br />
                  <strong>Altitude:</strong> {location.altitude.toFixed(1)} m
                </p>
              </div>
            ))
          ) : (
            <p>No location history available for the last 24 hours.</p>
          )}
        </div>
      </div>
      
      <div className="location-info">
        <h3>About Location Tracking</h3>
        <p>
          The wearable device uses GPS technology to track your location in real-time.
          This data can be useful for tracking outdoor activities, navigation, and emergency situations.
        </p>
        <p>
          The GPS 511-TESEO-LIV3R sensor in your wearable device provides accurate positioning data
          with minimal power consumption to maximize battery life.
        </p>
      </div>
    </div>
  );
};

export default Location;
