import React, { useEffect, useState } from 'react';
import { collection, query, where, orderBy, limit, onSnapshot } from 'firebase/firestore';
import { db } from '../firebase';
import LineChart from '../components/LineChart';
import './HeartRate.css';

const HeartRate = ({ userId }) => {
  const [heartRateData, setHeartRateData] = useState({
    current: { bpm: 0, timestamp: '' },
    history: [],
    dailyAvg: 0,
    min: 0,
    max: 0
  });
  
  useEffect(() => {
    // Get current heart rate
    const userDocRef = db.collection('users').doc(userId);
    
    const unsubscribe1 = onSnapshot(userDocRef, (docSnap) => {
      if (docSnap.exists()) {
        const data = docSnap.data();
        if (data.latest_heart_rate) {
          setHeartRateData(prev => ({
            ...prev,
            current: data.latest_heart_rate
          }));
        }
      }
    });
    
    // Get heart rate history for the last 24 hours
    const now = new Date();
    const yesterday = new Date(now.getTime() - (24 * 60 * 60 * 1000));
    
    const heartRateCollectionRef = collection(db, 'users', userId, 'heart_rate');
    const heartRateQuery = query(
      heartRateCollectionRef,
      where('timestamp', '>=', yesterday.toISOString()),
      orderBy('timestamp', 'asc')
    );
    
    const unsubscribe2 = onSnapshot(heartRateQuery, (querySnapshot) => {
      const heartRateHistory = [];
      let sum = 0;
      let min = 999;
      let max = 0;
      
      querySnapshot.forEach((doc) => {
        const data = doc.data();
        const bpm = data.bpm;
        
        heartRateHistory.push({
          timestamp: new Date(data.timestamp).toLocaleTimeString(),
          value: bpm
        });
        
        sum += bpm;
        min = Math.min(min, bpm);
        max = Math.max(max, bpm);
      });
      
      const avg = heartRateHistory.length > 0 ? Math.round(sum / heartRateHistory.length) : 0;
      
      setHeartRateData(prev => ({
        ...prev,
        history: heartRateHistory,
        dailyAvg: avg,
        min: min === 999 ? 0 : min,
        max: max
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
  
  const getHeartRateZone = (bpm) => {
    if (bpm < 60) return { zone: 'Resting', color: '#5e9fff' };
    if (bpm < 100) return { zone: 'Normal', color: '#5eff8f' };
    if (bpm < 140) return { zone: 'Moderate', color: '#ffde5e' };
    if (bpm < 170) return { zone: 'Intense', color: '#ff9d5e' };
    return { zone: 'Maximum', color: '#ff5e5e' };
  };
  
  const heartRateZone = getHeartRateZone(heartRateData.current.bpm);
  
  return (
    <div className="heart-rate-page">
      <h1>Heart Rate Monitoring</h1>
      
      <div className="current-hr-container">
        <div className="current-hr" style={{ borderColor: heartRateZone.color }}>
          <h2>{heartRateData.current.bpm}</h2>
          <p>BPM</p>
        </div>
        <div className="hr-details">
          <p className="hr-zone" style={{ color: heartRateZone.color }}>
            {heartRateZone.zone} Zone
          </p>
          <p>Last updated: {formatTimestamp(heartRateData.current.timestamp)}</p>
        </div>
      </div>
      
      <div className="hr-stats">
        <div className="stat-box">
          <h3>Daily Average</h3>
          <p>{heartRateData.dailyAvg} BPM</p>
        </div>
        <div className="stat-box">
          <h3>Minimum</h3>
          <p>{heartRateData.min} BPM</p>
        </div>
        <div className="stat-box">
          <h3>Maximum</h3>
          <p>{heartRateData.max} BPM</p>
        </div>
      </div>
      
      <div className="hr-chart-container">
        <h2>24-Hour Heart Rate</h2>
        <LineChart 
          data={heartRateData.history} 
          dataKey="value"
          xAxisDataKey="timestamp"
          color="#ff5e5e"
        />
      </div>
      
      <div className="hr-info">
        <h3>About Heart Rate</h3>
        <p>
          Your heart rate is the number of times your heart beats per minute (BPM).
          A normal resting heart rate for adults ranges from 60 to 100 BPM.
          Athletes and people with high cardiovascular fitness may have resting heart rates as low as 40 BPM.
        </p>
        <h4>Heart Rate Zones:</h4>
        <ul>
          <li><span style={{ color: '#5e9fff' }}>Resting:</span> Below 60 BPM</li>
          <li><span style={{ color: '#5eff8f' }}>Normal:</span> 60-100 BPM</li>
          <li><span style={{ color: '#ffde5e' }}>Moderate:</span> 100-140 BPM</li>
          <li><span style={{ color: '#ff9d5e' }}>Intense:</span> 140-170 BPM</li>
          <li><span style={{ color: '#ff5e5e' }}>Maximum:</span> Above 170 BPM</li>
        </ul>
      </div>
    </div>
  );
};

export default HeartRate;
