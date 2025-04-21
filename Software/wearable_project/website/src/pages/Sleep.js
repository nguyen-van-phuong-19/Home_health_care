import React, { useEffect, useState } from 'react';
import { collection, query, where, orderBy, limit, onSnapshot } from 'firebase/firestore';
import { db } from '../firebase';
import LineChart from '../components/LineChart';
import './Sleep.css';

const Sleep = ({ userId }) => {
  const [sleepData, setSleepData] = useState({
    latest: { start_time: '', end_time: '', duration_hours: 0 },
    weekly: [],
    average: 0
  });
  
  useEffect(() => {
    // Get latest sleep data
    const userDocRef = db.collection('users').doc(userId);
    
    const unsubscribe1 = onSnapshot(userDocRef, (docSnap) => {
      if (docSnap.exists()) {
        const data = docSnap.data();
        if (data.latest_sleep) {
          setSleepData(prev => ({
            ...prev,
            latest: data.latest_sleep
          }));
        }
      }
    });
    
    // Get sleep history for the last 7 days
    const now = new Date();
    const sevenDaysAgo = new Date(now.getTime() - (7 * 24 * 60 * 60 * 1000));
    
    const sleepCollectionRef = collection(db, 'users', userId, 'sleep');
    const sleepQuery = query(
      sleepCollectionRef,
      where('start_time', '>=', sevenDaysAgo.toISOString()),
      orderBy('start_time', 'asc')
    );
    
    const unsubscribe2 = onSnapshot(sleepQuery, (querySnapshot) => {
      const sleepHistory = [];
      let totalDuration = 0;
      
      querySnapshot.forEach((doc) => {
        const data = doc.data();
        const startDate = new Date(data.start_time).toLocaleDateString();
        
        sleepHistory.push({
          date: startDate,
          value: data.duration_hours
        });
        
        totalDuration += data.duration_hours;
      });
      
      const avgDuration = sleepHistory.length > 0 ? totalDuration / sleepHistory.length : 0;
      
      setSleepData(prev => ({
        ...prev,
        weekly: sleepHistory,
        average: avgDuration.toFixed(1)
      }));
    });
    
    return () => {
      unsubscribe1();
      unsubscribe2();
    };
  }, [userId]);
  
  const formatDateTime = (timestamp) => {
    if (!timestamp) return 'N/A';
    return new Date(timestamp).toLocaleString();
  };
  
  const getSleepQuality = (hours) => {
    if (hours < 6) return { quality: 'Poor', color: '#ff5e5e' };
    if (hours < 7) return { quality: 'Fair', color: '#ffde5e' };
    if (hours < 9) return { quality: 'Good', color: '#5eff8f' };
    return { quality: 'Excellent', color: '#5e9fff' };
  };
  
  const sleepQuality = getSleepQuality(sleepData.latest.duration_hours);
  
  return (
    <div className="sleep-page">
      <h1>Sleep Tracking</h1>
      
      <div className="latest-sleep-container">
        <div className="latest-sleep" style={{ borderColor: sleepQuality.color }}>
          <h2>{sleepData.latest.duration_hours.toFixed(1)}</h2>
          <p>hours</p>
        </div>
        <div className="sleep-details">
          <p className="sleep-quality" style={{ color: sleepQuality.color }}>
            {sleepQuality.quality} Quality
          </p>
          <p>From: {formatDateTime(sleepData.latest.start_time)}</p>
          <p>To: {formatDateTime(sleepData.latest.end_time)}</p>
        </div>
      </div>
      
      <div className="sleep-stats">
        <div className="stat-box">
          <h3>Weekly Average</h3>
          <p>{sleepData.average} hours</p>
        </div>
        <div className="stat-box">
          <h3>Recommended</h3>
          <p>7-9 hours</p>
        </div>
      </div>
      
      <div className="sleep-chart-container">
        <h2>Weekly Sleep Duration</h2>
        <LineChart 
          data={sleepData.weekly} 
          dataKey="value"
          xAxisDataKey="date"
          color="#c45eff"
        />
      </div>
      
      <div className="sleep-info">
        <h3>About Sleep Tracking</h3>
        <p>
          Sleep tracking uses your heart rate and movement patterns to estimate when you fall asleep and wake up.
          The wearable device monitors these patterns to calculate your total sleep duration.
        </p>
        <h4>Sleep Quality Guidelines:</h4>
        <ul>
          <li><span style={{ color: '#ff5e5e' }}>Poor:</span> Less than 6 hours</li>
          <li><span style={{ color: '#ffde5e' }}>Fair:</span> 6-7 hours</li>
          <li><span style={{ color: '#5eff8f' }}>Good:</span> 7-9 hours</li>
          <li><span style={{ color: '#5e9fff' }}>Excellent:</span> More than 9 hours</li>
        </ul>
        <p>
          Most adults need 7-9 hours of quality sleep per night for optimal health.
          Consistent sleep patterns help improve overall well-being and cognitive function.
        </p>
      </div>
    </div>
  );
};

export default Sleep;
