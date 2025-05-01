import React, { useEffect, useState } from 'react';
import { collection, query, where, orderBy, limit, onSnapshot } from 'firebase/firestore';
import { db } from '../firebase';
import LineChart from '../components/LineChart';
import './SpO2.css';

const SpO2 = ({ userId }) => {
  const [spo2Data, setSpo2Data] = useState({
    current: { percentage: 0, timestamp: '' },
    history: [],
    dailyAvg: 0,
    min: 0,
    max: 0
  });
  
  useEffect(() => {
    // Get current SpO2
    const userDocRef = db.collection('users').doc(userId);
    
    const unsubscribe1 = onSnapshot(userDocRef, (docSnap) => {
      if (docSnap.exists()) {
        const data = docSnap.data();
        if (data.latest_spo2) {
          setSpo2Data(prev => ({
            ...prev,
            current: data.latest_spo2
          }));
        }
      }
    });
    
    // Get SpO2 history for the last 24 hours
    const now = new Date();
    const yesterday = new Date(now.getTime() - (24 * 60 * 60 * 1000));
    
    const spo2CollectionRef = collection(db, 'users', userId, 'spo2');
    const spo2Query = query(
      spo2CollectionRef,
      where('timestamp', '>=', yesterday.toISOString()),
      orderBy('timestamp', 'asc')
    );
    
    const unsubscribe2 = onSnapshot(spo2Query, (querySnapshot) => {
      const spo2History = [];
      let sum = 0;
      let min = 101;
      let max = 0;
      
      querySnapshot.forEach((doc) => {
        const data = doc.data();
        const percentage = data.percentage;
        
        spo2History.push({
          timestamp: new Date(data.timestamp).toLocaleTimeString(),
          value: percentage
        });
        
        sum += percentage;
        min = Math.min(min, percentage);
        max = Math.max(max, percentage);
      });
      
      const avg = spo2History.length > 0 ? Math.round(sum / spo2History.length) : 0;
      
      setSpo2Data(prev => ({
        ...prev,
        history: spo2History,
        dailyAvg: avg,
        min: min === 101 ? 0 : min,
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
  
  const getSpo2Status = (percentage) => {
    if (percentage < 90) return { status: 'Low', color: '#ff5e5e' };
    if (percentage < 95) return { status: 'Acceptable', color: '#ffde5e' };
    return { status: 'Normal', color: '#5eff8f' };
  };
  
  const spo2Status = getSpo2Status(spo2Data.current.percentage);
  
  return (
    <div className="spo2-page">
      <h1>Blood Oxygen (SpO2) Monitoring</h1>
      
      <div className="current-spo2-container">
        <div className="current-spo2" style={{ borderColor: spo2Status.color }}>
          <h2>{spo2Data.current.percentage}</h2>
          <p>%</p>
        </div>
        <div className="spo2-details">
          <p className="spo2-status" style={{ color: spo2Status.color }}>
            {spo2Status.status}
          </p>
          <p>Last updated: {formatTimestamp(spo2Data.current.timestamp)}</p>
        </div>
      </div>
      
      <div className="spo2-stats">
        <div className="stat-box">
          <h3>Daily Average</h3>
          <p>{spo2Data.dailyAvg}%</p>
        </div>
        <div className="stat-box">
          <h3>Minimum</h3>
          <p>{spo2Data.min}%</p>
        </div>
        <div className="stat-box">
          <h3>Maximum</h3>
          <p>{spo2Data.max}%</p>
        </div>
      </div>
      
      <div className="spo2-chart-container">
        <h2>24-Hour SpO2 Levels</h2>
        <LineChart 
          data={spo2Data.history} 
          dataKey="value"
          xAxisDataKey="timestamp"
          color="#5e9fff"
        />
      </div>
      
      <div className="spo2-info">
        <h3>About Blood Oxygen Saturation (SpO2)</h3>
        <p>
          SpO2 is a measure of the amount of oxygen-carrying hemoglobin in the blood relative to the amount of hemoglobin not carrying oxygen.
          Normal SpO2 levels are typically between 95% and 100%.
        </p>
        <h4>SpO2 Levels:</h4>
        <ul>
          <li><span style={{ color: '#5eff8f' }}>Normal:</span> 95-100%</li>
          <li><span style={{ color: '#ffde5e' }}>Acceptable:</span> 90-94%</li>
          <li><span style={{ color: '#ff5e5e' }}>Low:</span> Below 90% (may require medical attention)</li>
        </ul>
        <p>
          Low SpO2 levels can indicate hypoxemia (low blood oxygen), which may be caused by various conditions including respiratory disorders, sleep apnea, or high altitude.
        </p>
      </div>
    </div>
  );
};

export default SpO2;
