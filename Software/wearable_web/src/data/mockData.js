
const mockData = {
  heart_rate: {
    '2025-04-30T10:00:00Z': { bpm: 72 },
    '2025-04-30T10:05:00Z': { bpm: 75 }
  },
  spo2: {
    '2025-04-30T10:00:00Z': { percentage: 98 },
    '2025-04-30T10:05:00Z': { percentage: 97 }
  },
  calories_by_heart_rate: {
    '2025-04-30': { calories_burned_hr: 120 }
  },
  calories_by_accelerometer: {
    '2025-04-30': { total_calories: 300 }
  },
  calories_daily: {
    '2025-04-30': { combined_daily_calories: 420 }
  },
  daily_sleep: {
    '2025-04-30': { sleep_duration: 7.5 }
  },
  sleep: {
    '2025-04-29T22:00:00Z': {
      duration_hours: 7.5,
      start_time: '2025-04-29T22:00:00Z',
      end_time: '2025-04-30T05:30:00Z'
    }
  },
  latest_location: {
    latitude: 13.736717,
    longitude: 100.523186
  }
};

export default mockData;