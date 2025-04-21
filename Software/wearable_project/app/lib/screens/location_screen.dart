import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:provider/provider.dart';
import 'package:wearable_app/services/user_provider.dart';
import 'package:google_maps_flutter/google_maps_flutter.dart';
import 'package:intl/intl.dart';

class LocationScreen extends StatefulWidget {
  const LocationScreen({Key? key}) : super(key: key);

  @override
  State<LocationScreen> createState() => _LocationScreenState();
}

class _LocationScreenState extends State<LocationScreen> {
  Map<String, dynamic> locationData = {
    'current': {'latitude': 0, 'longitude': 0, 'altitude': 0, 'timestamp': ''},
    'history': []
  };
  
  GoogleMapController? _mapController;
  Set<Marker> _markers = {};
  Set<Polyline> _polylines = {};

  @override
  void initState() {
    super.initState();
    _fetchLocationData();
    _fetchLocationHistory();
  }

  void _fetchLocationData() {
    final userId = Provider.of<UserProvider>(context, listen: false).userId;
    
    FirebaseFirestore.instance
        .collection('users')
        .doc(userId)
        .snapshots()
        .listen((docSnapshot) {
      if (docSnapshot.exists) {
        final data = docSnapshot.data()!;
        if (data['latest_location'] != null) {
          setState(() {
            locationData['current'] = data['latest_location'];
          });
          
          _updateMapView();
        }
      }
    });
  }

  void _fetchLocationHistory() {
    final userId = Provider.of<UserProvider>(context, listen: false).userId;
    
    // Get location history for the last 24 hours
    final now = DateTime.now();
    final yesterday = now.subtract(const Duration(hours: 24));
    
    FirebaseFirestore.instance
        .collection('users')
        .doc(userId)
        .collection('gps')
        .where('timestamp', isGreaterThanOrEqualTo: yesterday.toIso8601String())
        .orderBy('timestamp')
        .snapshots()
        .listen((querySnapshot) {
      final locationHistory = querySnapshot.docs.map((doc) {
        final data = doc.data();
        return {
          'latitude': data['latitude'],
          'longitude': data['longitude'],
          'altitude': data['altitude'],
          'timestamp': data['timestamp']
        };
      }).toList();
      
      setState(() {
        locationData['history'] = locationHistory;
      });
      
      _updateMapView();
    });
  }

  void _updateMapView() {
    if (_mapController == null) return;
    
    // Create markers
    final markers = <Marker>{};
    final polylinePoints = <LatLng>[];
    
    // Current location marker
    final currentLat = locationData['current']['latitude'] as double? ?? 0;
    final currentLng = locationData['current']['longitude'] as double? ?? 0;
    
    if (currentLat != 0 && currentLng != 0) {
      final currentLatLng = LatLng(currentLat, currentLng);
      
      markers.add(
        Marker(
          markerId: const MarkerId('current_location'),
          position: currentLatLng,
          infoWindow: InfoWindow(
            title: 'Current Location',
            snippet: _formatTimestamp(locationData['current']['timestamp'] ?? ''),
          ),
          icon: BitmapDescriptor.defaultMarkerWithHue(BitmapDescriptor.hueBlue),
        ),
      );
      
      polylinePoints.add(currentLatLng);
      
      // Move camera to current location
      _mapController?.animateCamera(
        CameraUpdate.newLatLngZoom(currentLatLng, 15),
      );
    }
    
    // History markers
    final history = locationData['history'] as List;
    for (int i = 0; i < history.length; i++) {
      final location = history[i];
      final lat = location['latitude'] as double? ?? 0;
      final lng = location['longitude'] as double? ?? 0;
      
      if (lat != 0 && lng != 0) {
        final latLng = LatLng(lat, lng);
        
        markers.add(
          Marker(
            markerId: MarkerId('history_$i'),
            position: latLng,
            infoWindow: InfoWindow(
              title: 'Location History',
              snippet: _formatTimestamp(location['timestamp'] ?? ''),
            ),
            icon: BitmapDescriptor.defaultMarkerWithHue(BitmapDescriptor.hueRed),
            alpha: 0.7,
          ),
        );
        
        polylinePoints.add(latLng);
      }
    }
    
    // Create polyline
    final polylines = <Polyline>{};
    if (polylinePoints.length > 1) {
      polylines.add(
        Polyline(
          polylineId: const PolylineId('route'),
          points: polylinePoints,
          color: Colors.blue,
          width: 5,
        ),
      );
    }
    
    setState(() {
      _markers = markers;
      _polylines = polylines;
    });
  }

  String _formatTimestamp(String timestamp) {
    if (timestamp.isEmpty) return 'N/A';
    try {
      final dateTime = DateTime.parse(timestamp);
      return DateFormat.yMd().add_jm().format(dateTime);
    } catch (e) {
      return 'Invalid date';
    }
  }

  @override
  Widget build(BuildContext context) {
    final currentLat = locationData['current']['latitude'] as double? ?? 0;
    final currentLng = locationData['current']['longitude'] as double? ?? 0;
    final currentAlt = locationData['current']['altitude'] as double? ?? 0;
    
    // Default to a location if no data available
    final initialCameraPosition = CameraPosition(
      target: currentLat != 0 && currentLng != 0
          ? LatLng(currentLat, currentLng)
          : const LatLng(37.7749, -122.4194), // Default to San Francisco
      zoom: 15,
    );
    
    return Scaffold(
      appBar: AppBar(
        title: const Text('Location Tracking'),
      ),
      body: Column(
        children: [
          // Current location details
          Card(
            margin: const EdgeInsets.all(16),
            child: Padding(
              padding: const EdgeInsets.all(16),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  const Text(
                    'Current Location',
                    style: TextStyle(
                      fontSize: 18,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  const SizedBox(height: 16),
                  Row(
                    children: [
                      Expanded(
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            const Text(
                              'Latitude',
                              style: TextStyle(
                                fontWeight: FontWeight.bold,
                              ),
                            ),
                            Text('${currentLat.toStringAsFixed(6)}°'),
                          ],
                        ),
                      ),
                      Expanded(
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            const Text(
                              'Longitude',
                              style: TextStyle(
                                fontWeight: FontWeight.bold,
                              ),
                            ),
                            Text('${currentLng.toStringAsFixed(6)}°'),
                          ],
                        ),
                      ),
                      Expanded(
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            const Text(
                              'Altitude',
                              style: TextStyle(
                                fontWeight: FontWeight.bold,
                              ),
                            ),
                            Text('${currentAlt.toStringAsFixed(1)} m'),
                          ],
                        ),
                      ),
                    ],
                  ),
                  const SizedBox(height: 8),
                  Text(
                    'Last updated: ${_formatTimestamp(locationData['current']['timestamp'] ?? '')}',
                    style: TextStyle(
                      fontSize: 12,
                      color: Colors.grey[600],
                    ),
                  ),
                ],
              ),
            ),
          ),
          
          // Map
          Expanded(
            child: GoogleMap(
              initialCameraPosition: initialCameraPosition,
              markers: _markers,
              polylines: _polylines,
              mapType: MapType.normal,
              myLocationEnabled: true,
              myLocationButtonEnabled: true,
              zoomControlsEnabled: true,
              onMapCreated: (controller) {
                setState(() {
                  _mapController = controller;
                });
                _updateMapView();
              },
            ),
          ),
          
          // Location history list
          Container(
            height: 150,
            padding: const EdgeInsets.all(16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                const Text(
                  'Location History',
                  style: TextStyle(
                    fontSize: 16,
                    fontWeight: FontWeight.bold,
                  ),
                ),
                const SizedBox(height: 8),
                Expanded(
                  child: (locationData['history'] as List).isEmpty
                      ? const Center(child: Text('No location history available'))
                      : ListView.builder(
                          itemCount: (locationData['history'] as List).length,
                          itemBuilder: (context, index) {
                            final location = (locationData['history'] as List)[index];
                            return ListTile(
                              dense: true,
                              title: Text(
                                '${location['latitude'].toStringAsFixed(6)}°, ${location['longitude'].toStringAsFixed(6)}°',
                                style: const TextStyle(fontSize: 14),
                              ),
                              subtitle: Text(
                                _formatTimestamp(location['timestamp']),
                                style: const TextStyle(fontSize: 12),
                              ),
                              onTap: () {
                                final lat = location['latitude'] as double;
                                final lng = location['longitude'] as double;
                                _mapController?.animateCamera(
                                  CameraUpdate.newLatLngZoom(LatLng(lat, lng), 15),
                                );
                              },
                            );
                          },
                        ),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}
