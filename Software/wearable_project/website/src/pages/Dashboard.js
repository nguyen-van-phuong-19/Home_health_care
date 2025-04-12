import React from 'react';
import './Dashboard.css';
import { useEffect, useState } from 'react';
import { collection, doc, getDoc, onSnapshot } from 'firebase/firestore';
import { db } from '../firebase';
import StatCard from '../components/StatCard';
import LineChart from '../components/LineChart';
import { FaHeartbeat, FaRunning, FaBed } from 'react-icons/fa';
import { GiLungs } from 'react-icons/gi';

const Dashboard = ({ userId }) => {
  const [userData, setUserData] = useState({
    heartRate: { bpm: 0, timestamp: '' },
    spo2: { percentage: 0, timestamp: '' },
    calories: { burned_today: 0, timestamp: '' },
    sleep: { duration_hours: 0, timestamp: '' },
    location: { latitude: 0, longitude: 0, timestamp: '' }
  });
  
  const [heartRateHistory, setHeartRateHistory] = useState([]);
  
  useEffect(() => {
    // Get user document with latest values
    const userDocRef = doc(db, 'users', userId);
    
    const unsubscribe = onSnapshot(userDocRef, (docSnap) => {
      if (docSnap.exists()) {
        const data = docSnap.data();
        setUserData({
          heartRate: data.latest_heart_rate || { bpm: 0, timestamp: '' },
          spo2: data.latest_spo2 || { percentage: 0, timestamp: '' },
          calories: data.latest_activity || { calories_burned_today: 0, timestamp: '' },
          sleep: data.latest_sleep || { duration_hours: 0, timestamp: '' },
          location: data.latest_location || { latitude: 0, longitude: 0, timestamp: '' }
        });
      }
    });
    
    // Get heart rate history for the chart
    const today = new Date();
    const startOfDay = new Date(today.setHours(0, 0, 0, 0)).toISOString();
    
    const heartRateCollectionRef = collection(db, 'users', userId, 'heart_rate');
    const heartRateQuery = heartRateCollectionRef.where('timestamp', '>=', startOfDay);
    
    const heartRateUnsubscribe = onSnapshot(heartRateQuery, (querySnapshot) => {
      const heartRateData = [];
      querySnapshot.forEach((doc) => {
        const data = doc.data();
        heartRateData.push({
          timestamp: new Date(data.timestamp).toLocaleTimeString(),
          value: data.bpm
        });
      });
      
      // Sort by timestamp
      heartRateData.sort((a, b) => new Date(a.timestamp) - new Date(b.timestamp));
      setHeartRateHistory(heartRateData);
    });
    
    return () => {
      unsubscribe();
      heartRateUnsubscribe();
    };
  }, [userId]);
  
  const formatTimestamp = (timestamp) => {
    if (!timestamp) return 'N/A';
    return new Date(timestamp).toLocaleTimeString();
  };
  
  return (
    <div className="dashboard">
      <h1>Health Dashboard</h1>
      <p className="last-updated">Last updated: {formatTimestamp(userData.heartRate.timestamp)}</p>
      
      <div className="stat-cards">
        <StatCard 
          title="Heart Rate" 
          value={userData.heartRate.bpm} 
          unit="bpm" 
          icon={<FaHeartbeat />} 
          color="#ff5e5e"
        />
        <StatCard 
          title="SpO2" 
          value={userData.spo2.percentage} 
          unit="%" 
          icon={<GiLungs />} 
          color="#5e9fff"
        />
        <StatCard 
          title="Calories" 
          value={userData.calories.calories_burned_today} 
          unit="kcal" 
          icon={<FaRunning />} 
          color="#5eff8f"
        />
        <StatCard 
          title="Sleep" 
          value={userData.sleep.duration_hours} 
          unit="hours" 
          icon={<FaBed />} 
          color="#c45eff"
        />
      </div>
      
      <div className="charts-container">
        <div className="chart-card">
          <h2>Heart Rate Today</h2>
          <LineChart 
            data={heartRateHistory} 
            dataKey="value"
            xAxisDataKey="timestamp"
            color="#ff5e5e"
          />
        </div>
      </div>
      
      <div className="location-card">
        <h2>Last Known Location</h2>
        <p>
          Latitude: {userData.location.latitude.toFixed(6)}, 
          Longitude: {userData.location.longitude.toFixed(6)}
        </p>
        <p>Recorded at: {formatTimestamp(userData.location.timestamp)}</p>
      </div>
    </div>
  );
};

export default Dashboard;
