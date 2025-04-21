import React, { useEffect, useState } from 'react';
import { collection, query, where, orderBy, limit, onSnapshot } from 'firebase/firestore';
import { db } from '../firebase';
import LineChart from '../components/LineChart';
import './Calories.css';

const Calories = ({ userId }) => {
  const [caloriesData, setCaloriesData] = useState({
    current: { calories_burned_today: 0, state: 'resting', timestamp: '' },
    history: [],
    weeklyData: {},
    goal: 2000 // Default goal
  });
  
  useEffect(() => {
    // Get current calories data
    const userDocRef = db.collection('users').doc(userId);
    
    const unsubscribe1 = onSnapshot(userDocRef, (docSnap) => {
      if (docSnap.exists()) {
        const data = docSnap.data();
        if (data.latest_activity) {
          setCaloriesData(prev => ({
            ...prev,
            current: data.latest_activity
          }));
        }
      }
    });
    
    // Get activity history for today
    const today = new Date();
    const startOfDay = new Date(today.setHours(0, 0, 0, 0)).toISOString();
    
    const activityCollectionRef = collection(db, 'users', userId, 'activity');
    const activityQuery = query(
      activityCollectionRef,
      where('timestamp', '>=', startOfDay),
      orderBy('timestamp', 'asc')
    );
    
    const unsubscribe2 = onSnapshot(activityQuery, (querySnapshot) => {
      const activityHistory = [];
      
      querySnapshot.forEach((doc) => {
        const data = doc.data();
        
        activityHistory.push({
          timestamp: new Date(data.timestamp).toLocaleTimeString(),
          value: data.calories_burned,
          activity: data.activity_state
        });
      });
      
      setCaloriesData(prev => ({
        ...prev,
        history: activityHistory
      }));
    });
    
    // Get weekly stats
    const now = new Date();
    const sevenDaysAgo = new Date(now.getTime() - (7 * 24 * 60 * 60 * 1000));
    
    const dailyStatsCollectionRef = collection(db, 'users', userId, 'daily_stats');
    const weeklyStatsQuery = query(
      dailyStatsCollectionRef,
      where('timestamp', '>=', sevenDaysAgo.toISOString()),
      orderBy('timestamp', 'asc')
    );
    
    const unsubscribe3 = onSnapshot(weeklyStatsQuery, (querySnapshot) => {
      const weeklyData = {};
      
      querySnapshot.forEach((doc) => {
        const data = doc.data();
        const date = doc.id; // Using the document ID as the date (YYYY-MM-DD)
        
        weeklyData[date] = {
          total_calories: data.total_calories || 0
        };
      });
      
      setCaloriesData(prev => ({
        ...prev,
        weeklyData: weeklyData
      }));
    });
    
    return () => {
      unsubscribe1();
      unsubscribe2();
      unsubscribe3();
    };
  }, [userId]);
  
  const formatTimestamp = (timestamp) => {
    if (!timestamp) return 'N/A';
    return new Date(timestamp).toLocaleString();
  };
  
  const getActivityColor = (activity) => {
    switch (activity) {
      case 'resting': return '#5e9fff';
      case 'light': return '#5eff8f';
      case 'moderate': return '#ffde5e';
      case 'vigorous': return '#ff5e5e';
      default: return '#5e9fff';
    }
  };
  
  const activityColor = getActivityColor(caloriesData.current.state);
  
  // Prepare weekly chart data
  const weeklyChartData = Object.keys(caloriesData.weeklyData).map(date => ({
    date: date,
    value: caloriesData.weeklyData[date].total_calories
  }));
  
  // Calculate progress percentage
  const progressPercentage = Math.min(
    Math.round((caloriesData.current.calories_burned_today / caloriesData.goal) * 100),
    100
  );
  
  return (
    <div className="calories-page">
      <h1>Calories Burned</h1>
      
      <div className="calories-overview">
        <div className="calories-progress-container">
          <div className="calories-progress">
            <svg viewBox="0 0 100 100" width="200" height="200">
              <circle
                cx="50"
                cy="50"
                r="45"
                fill="none"
                stroke="#e6e6e6"
                strokeWidth="10"
              />
              <circle
                cx="50"
                cy="50"
                r="45"
                fill="none"
                stroke="#5eff8f"
                strokeWidth="10"
                strokeDasharray="283"
                strokeDashoffset={283 - (283 * progressPercentage) / 100}
                transform="rotate(-90 50 50)"
              />
              <text x="50" y="50" textAnchor="middle" dy="7" fontSize="20">
                {caloriesData.current.calories_burned_today}
              </text>
              <text x="50" y="70" textAnchor="middle" dy="7" fontSize="12">
                of {caloriesData.goal} kcal
              </text>
            </svg>
          </div>
        </div>
        
        <div className="calories-details">
          <div className="current-activity">
            <h3>Current Activity</h3>
            <p className="activity-state" style={{ color: activityColor }}>
              {caloriesData.current.state.charAt(0).toUpperCase() + caloriesData.current.state.slice(1)}
            </p>
            <p>Last updated: {formatTimestamp(caloriesData.current.timestamp)}</p>
          </div>
        </div>
      </div>
      
      <div className="calories-charts">
        <div className="chart-container">
          <h2>Today's Calorie Burn</h2>
          <LineChart 
            data={caloriesData.history} 
            dataKey="value"
            xAxisDataKey="timestamp"
            color="#5eff8f"
          />
        </div>
        
        <div className="chart-container">
          <h2>Weekly Calories</h2>
          <LineChart 
            data={weeklyChartData} 
            dataKey="value"
            xAxisDataKey="date"
            color="#5eff8f"
          />
        </div>
      </div>
      
      <div className="calories-info">
        <h3>About Calorie Expenditure</h3>
        <p>
          Calorie expenditure is the amount of energy your body uses during physical activities and basic bodily functions.
          The wearable device calculates calories burned based on your heart rate, movement patterns, and personal metrics.
        </p>
        <h4>Activity Levels:</h4>
        <ul>
          <li><span style={{ color: '#5e9fff' }}>Resting:</span> Minimal movement, sitting or lying down</li>
          <li><span style={{ color: '#5eff8f' }}>Light:</span> Walking, light household chores</li>
          <li><span style={{ color: '#ffde5e' }}>Moderate:</span> Brisk walking, cycling at moderate pace</li>
          <li><span style={{ color: '#ff5e5e' }}>Vigorous:</span> Running, high-intensity workouts</li>
        </ul>
      </div>
    </div>
  );
};

export default Calories;
