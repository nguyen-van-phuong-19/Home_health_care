"""
Firebase Integration Module
--------------------------
This module provides helper functions for interacting with Firebase from the MQTT server,
React.js website, and Flutter app.
"""

import firebase_admin
from firebase_admin import credentials
from firebase_admin import firestore
import json
from datetime import datetime, timedelta

class FirebaseManager:
    """Class to manage Firebase operations for the wearable device ecosystem"""
    
    def __init__(self, credentials_path):
        """Initialize Firebase connection with the provided credentials"""
        self.cred = credentials.Certificate(credentials_path)
        firebase_admin.initialize_app(self.cred)
        self.db = firestore.client()
        print("Firebase connection initialized successfully")
    
    def store_sensor_data(self, user_id, data_type, data):
        """
        Store sensor data in Firebase
        
        Args:
            user_id (str): User identifier
            data_type (str): Type of data (heart_rate, spo2, activity, gps, etc.)
            data (dict): Data to store
        
        Returns:
            bool: Success status
        """
        try:
            # Ensure timestamp exists
            if "timestamp" not in data:
                data["timestamp"] = datetime.now().isoformat()
            
            # Store in historical collection
            self.db.collection("users").document(user_id).collection(data_type).document(data["timestamp"]).set(data)
            
            # Update latest values
            self.db.collection("users").document(user_id).set({
                f"latest_{data_type}": data
            }, merge=True)
            
            return True
        except Exception as e:
            print(f"Error storing {data_type} data: {e}")
            return False
    
    def update_daily_stats(self, user_id, date, stats):
        """
        Update daily statistics for a user
        
        Args:
            user_id (str): User identifier
            date (str): Date in YYYY-MM-DD format
            stats (dict): Statistics to update
        
        Returns:
            bool: Success status
        """
        try:
            daily_stats_ref = self.db.collection("users").document(user_id).collection("daily_stats").document(date)
            daily_stats_ref.set(stats, merge=True)
            return True
        except Exception as e:
            print(f"Error updating daily stats: {e}")
            return False
    
    def get_latest_data(self, user_id, data_type):
        """
        Get latest data for a specific type
        
        Args:
            user_id (str): User identifier
            data_type (str): Type of data (heart_rate, spo2, activity, etc.)
        
        Returns:
            dict: Latest data or None if not found
        """
        try:
            user_doc = self.db.collection("users").document(user_id).get()
            if user_doc.exists:
                user_data = user_doc.to_dict()
                return user_data.get(f"latest_{data_type}")
            return None
        except Exception as e:
            print(f"Error getting latest {data_type} data: {e}")
            return None
    
    def get_historical_data(self, user_id, data_type, start_time, end_time=None):
        """
        Get historical data for a specific type within a time range
        
        Args:
            user_id (str): User identifier
            data_type (str): Type of data (heart_rate, spo2, activity, etc.)
            start_time (str): Start time in ISO format
            end_time (str, optional): End time in ISO format. Defaults to current time.
        
        Returns:
            list: List of data points
        """
        try:
            if end_time is None:
                end_time = datetime.now().isoformat()
            
            query = (self.db.collection("users").document(user_id).collection(data_type)
                    .where("timestamp", ">=", start_time)
                    .where("timestamp", "<=", end_time)
                    .order_by("timestamp"))
            
            results = query.stream()
            data_points = [doc.to_dict() for doc in results]
            return data_points
        except Exception as e:
            print(f"Error getting historical {data_type} data: {e}")
            return []
    
    def get_daily_stats(self, user_id, date):
        """
        Get daily statistics for a user
        
        Args:
            user_id (str): User identifier
            date (str): Date in YYYY-MM-DD format
        
        Returns:
            dict: Daily statistics or empty dict if not found
        """
        try:
            stats_doc = self.db.collection("users").document(user_id).collection("daily_stats").document(date).get()
            if stats_doc.exists:
                return stats_doc.to_dict()
            return {}
        except Exception as e:
            print(f"Error getting daily stats: {e}")
            return {}
    
    def get_weekly_stats(self, user_id, end_date=None):
        """
        Get weekly statistics for a user
        
        Args:
            user_id (str): User identifier
            end_date (str, optional): End date in YYYY-MM-DD format. Defaults to today.
        
        Returns:
            dict: Weekly statistics with dates as keys
        """
        try:
            if end_date is None:
                end_date = datetime.now().strftime("%Y-%m-%d")
            
            end_date_obj = datetime.strptime(end_date, "%Y-%m-%d")
            start_date_obj = end_date_obj - timedelta(days=6)  # 7 days including end_date
            
            weekly_stats = {}
            current_date = start_date_obj
            
            while current_date <= end_date_obj:
                date_str = current_date.strftime("%Y-%m-%d")
                daily_stats = self.get_daily_stats(user_id, date_str)
                weekly_stats[date_str] = daily_stats
                current_date += timedelta(days=1)
            
            return weekly_stats
        except Exception as e:
            print(f"Error getting weekly stats: {e}")
            return {}
    
    def register_user(self, user_id, user_data):
        """
        Register a new user or update existing user data
        
        Args:
            user_id (str): User identifier
            user_data (dict): User data
        
        Returns:
            bool: Success status
        """
        try:
            self.db.collection("users").document(user_id).set(user_data, merge=True)
            return True
        except Exception as e:
            print(f"Error registering user: {e}")
            return False
    
    def get_user_profile(self, user_id):
        """
        Get user profile data
        
        Args:
            user_id (str): User identifier
        
        Returns:
            dict: User profile data or None if not found
        """
        try:
            user_doc = self.db.collection("users").document(user_id).get()
            if user_doc.exists:
                return user_doc.to_dict()
            return None
        except Exception as e:
            print(f"Error getting user profile: {e}")
            return None

# Example usage
if __name__ == "__main__":
    # Initialize Firebase manager
    firebase_manager = FirebaseManager("firebase_credentials.json")
    
    # Example: Store heart rate data
    heart_rate_data = {
        "bpm": 75,
        "timestamp": datetime.now().isoformat()
    }
    firebase_manager.store_sensor_data("test_user", "heart_rate", heart_rate_data)
    
    # Example: Update daily stats
    today = datetime.now().strftime("%Y-%m-%d")
    daily_stats = {
        "total_calories": 1250,
        "sleep_duration": 7.5,
        "average_heart_rate": 72
    }
    firebase_manager.update_daily_stats("test_user", today, daily_stats)
    
    # Example: Get latest heart rate
    latest_hr = firebase_manager.get_latest_data("test_user", "heart_rate")
    print(f"Latest heart rate: {latest_hr}")
