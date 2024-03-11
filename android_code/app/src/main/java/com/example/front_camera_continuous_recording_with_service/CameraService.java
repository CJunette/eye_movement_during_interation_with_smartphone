package com.example.front_camera_continuous_recording_with_service;

import android.Manifest;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.ImageFormat;
import android.graphics.PixelFormat;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.CamcorderProfile;
import android.media.Image;
import android.media.ImageReader;
import android.media.MediaRecorder;
import android.os.Build;
import android.os.Environment;
import android.os.IBinder;
import android.text.TextUtils;
import android.util.Log;
import android.util.Range;
import android.util.Size;
import android.util.SparseIntArray;
import android.view.LayoutInflater;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import android.view.WindowManager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.core.app.ActivityCompat;
import androidx.core.app.NotificationCompat;

import java.io.File;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class CameraService extends Service
{

    private final static String TAG = "CameraService";

    public final static String ACTION_START = "com.compal.camera.start";
    public final static String ACTION_STOPPED = "com.compal.camera.stop";

    private final static int ONGOING_NOTIFICATION_ID = 5566;
    private final static String CHANNEL_ID = "cam_service_channel_id";
    private final static String CHANNEL_NAME = "cam_service_channel_name";

    public static boolean SHOW_PREVIEW = false;
    //For preview
    private WindowManager mWindowManager;
    private View mRootView;
    private TextureView mTextureView;

    private ImageReader mImageReader;
    private Size mPreviewSize;
    public MediaRecorder mMediaRecorder;

    private CameraManager mCameraManager;
    private CameraDevice mCameraDevice;
    private CaptureRequest.Builder mRequestBuilder;
    private CameraCaptureSession mCameraCaptureSession;

    private CameraCharacteristics mCharacteristics;

    private static final SparseIntArray ORIENTATIONS = new SparseIntArray();
    static
    {
        ORIENTATIONS.append(Surface.ROTATION_0, 90);
        ORIENTATIONS.append(Surface.ROTATION_90, 0);
        ORIENTATIONS.append(Surface.ROTATION_180, 270);
        ORIENTATIONS.append(Surface.ROTATION_270, 180);
    }

    private final CameraCaptureSession.CaptureCallback mCaptureCallback = new CameraCaptureSession.CaptureCallback() {

    };

    private final ImageReader.OnImageAvailableListener mImageListener = new ImageReader.OnImageAvailableListener()
    {
        @Override
        public void onImageAvailable(ImageReader reader)
        {
            Image image = reader.acquireLatestImage();
            if (null == image) {
                return;
            }
            Log.d(TAG, "onImageAvailable: " + image.getWidth() + " x " + image.getHeight());
            ByteBuffer buffer = image.getPlanes()[0].getBuffer();
            byte[] bytes = new byte[buffer.capacity()];
            buffer.get(bytes);

            image.close();
        }
    };

    private final CameraDevice.StateCallback mStateCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice camera) {
            mCameraDevice = camera;
            createCaptureSession();
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice camera) {
            camera.close();
            mCameraDevice = null;
        }

        @Override
        public void onError(@NonNull CameraDevice camera, int error) {
            camera.close();
            mCameraDevice = null;
        }
    };


    @RequiresApi(api = Build.VERSION_CODES.O)
    private void startForeground() {
        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, new Intent(this, MainActivity.class), PendingIntent.FLAG_UPDATE_CURRENT);

        NotificationChannel channel = new NotificationChannel(CHANNEL_ID, CHANNEL_NAME, NotificationManager.IMPORTANCE_NONE);
        channel.setLightColor(Color.BLUE);
        channel.setLockscreenVisibility(Notification.VISIBILITY_PRIVATE);
        NotificationManager notificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.createNotificationChannel(channel);

        Notification notification = new NotificationCompat.Builder(this, CHANNEL_ID)
                .setContentTitle(getText(R.string.app_name))
                .setContentText(getText(R.string.app_name))
                .setSmallIcon(R.drawable.ic_launcher_foreground)
                .setContentIntent(pendingIntent)
                .setTicker(getText(R.string.app_name))
                .build();

        startForeground(ONGOING_NOTIFICATION_ID, notification);
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent)
    {
        return null;
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    @Override
    public void onCreate() {
        super.onCreate();
        startForeground();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        stopCamera();
        if (mRootView != null) {
            mWindowManager.removeView(mRootView);
            mRootView = null;
        }
        sendBroadcast(new Intent(ACTION_STOPPED));
    }

    private final TextureView.SurfaceTextureListener surfaceTextureListener = new TextureView.SurfaceTextureListener() {

        @Override
        public void onSurfaceTextureAvailable(@NonNull SurfaceTexture surface, int width, int height) {
            initCam();
        }

        @Override
        public void onSurfaceTextureSizeChanged(@NonNull SurfaceTexture surface, int width, int height) {

        }

        @Override
        public boolean onSurfaceTextureDestroyed(@NonNull SurfaceTexture surface) {
            return true;
        }

        @Override
        public void onSurfaceTextureUpdated(@NonNull SurfaceTexture surface) {

        }
    };

    @Override
    public int onStartCommand(@Nullable Intent intent, int flags, int startId) {
        mWindowManager = (WindowManager) getSystemService(Context.WINDOW_SERVICE);

//        String action = intent.getAction();
//        String actionState = action.split("_")[0];
//        String checkBoxState = action.split("_")[1];
//        SHOW_PREVIEW = checkBoxState.equals("true");

        if (SHOW_PREVIEW) {
            // Initialize view drawn over other apps
            initOverlay();
            if (mTextureView.isAvailable()) {
                initCam();
            } else {
                mTextureView.setSurfaceTextureListener(surfaceTextureListener);
            }
        } else {
            initCam();
        }
        return super.onStartCommand(intent, flags, startId);
    }

    private void initOverlay() {
        mRootView = LayoutInflater.from(getApplicationContext()).inflate(R.layout.overlay, null);
        mTextureView = mRootView.findViewById(R.id.text_preview);

        WindowManager.LayoutParams params = new WindowManager.LayoutParams(
                WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY,
                WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
                PixelFormat.TRANSLUCENT
        );

        mWindowManager.addView(mRootView, params);
    }

    private void initCam() {
        mCameraManager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        String camId = null;
        try {
            for (String id : mCameraManager.getCameraIdList()) {
                mCharacteristics = mCameraManager.getCameraCharacteristics(id);
                int facing = mCharacteristics.get(CameraCharacteristics.LENS_FACING);
                if (facing == CameraCharacteristics.LENS_FACING_FRONT) {
                    camId = id;
                    break;
                }
            }

            if (!TextUtils.isEmpty(camId)) {
                mPreviewSize = chooseSupportedSize(camId);
                if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
                    return;
                }
                mCameraManager.openCamera(camId, mStateCallback, null);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private Size chooseSupportedSize(String camId) throws CameraAccessException
    {
        // Get all supported sizes for TextureView
        StreamConfigurationMap map = mCharacteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
//        Size[] supportedSizes = map.getOutputSizes(SurfaceTexture.class);

        List<Size> normalVideoSizes = new ArrayList<>();
        Range<Integer>[] aeFpsRanges = mCharacteristics.get(CameraCharacteristics.CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES);
        Size[] supportedSizes = map.getOutputSizes(MediaRecorder.class);

        for (Range<Integer> fpsRange : aeFpsRanges)
        {
            if (fpsRange.getLower().equals(fpsRange.getUpper()))
            {
                for (android.util.Size size : supportedSizes)
                {
                    Size videoSize = new Size(size.getWidth(), size.getHeight());
                    if (hasHighSpeedCamcorder(videoSize, CameraCharacteristics.LENS_FACING_FRONT))
                    {
                        Log.d(TAG, "Support HighSpeed video recording for " + videoSize.toString() + " fps: " + fpsRange.getUpper());
                        normalVideoSizes.add(videoSize);
                    }
                }
            }
        }

        if (normalVideoSizes.size() > 0)
        {
            // sort normalVideoSizes by area
            Collections.sort(normalVideoSizes, new Comparator<Size>() {
                @Override
                public int compare(Size o1, Size o2) {
                    return o1.getWidth() * o1.getHeight() - o2.getWidth() * o2.getHeight();
                }
            });
            return normalVideoSizes.get(normalVideoSizes.size() - 1);
        }

//        if (supportedSizes.length > 0) {
//            return supportedSizes[0];
//        }

        return new Size(320, 240);
    }

    private void createCaptureSession() {
        try {
            mRequestBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            // Set some additional parameters for the request
            mRequestBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
            mRequestBuilder.set(CaptureRequest.CONTROL_AE_MODE, CaptureRequest.CONTROL_AE_MODE_ON_AUTO_FLASH);

            // Prepare surfaces we want to use in capture session
            List<Surface> targetSurfaces = new ArrayList<>();

            // Configure target surface for background processing (ImageReader)
            mImageReader = ImageReader.newInstance(mPreviewSize.getWidth(), mPreviewSize.getHeight(), ImageFormat.YUV_420_888, 2);
            mImageReader.setOnImageAvailableListener(mImageListener, null);
            targetSurfaces.add(mImageReader.getSurface());
            mRequestBuilder.addTarget(mImageReader.getSurface());

            if (SHOW_PREVIEW) {
                SurfaceTexture texture = mTextureView.getSurfaceTexture();
                texture.setDefaultBufferSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());
                Surface previewSurface = new Surface(texture);
                targetSurfaces.add(previewSurface);
                mRequestBuilder.addTarget(previewSurface);
            }

            mMediaRecorder = new MediaRecorder();
            CamcorderProfile profile = getCamcorderProfile();
            mMediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
            mMediaRecorder.setVideoSource(MediaRecorder.VideoSource.SURFACE);
            mMediaRecorder.setProfile(profile);
            String nextVideoFilePath = getVideoFile();
            mMediaRecorder.setOutputFile(nextVideoFilePath);
            int rotation = mWindowManager.getDefaultDisplay().getRotation();
            int orientation = ORIENTATIONS.get(rotation);
            // 对于前置摄像头，需要额外检查镜像和旋转
            Integer frontFacing = mCharacteristics.get(CameraCharacteristics.LENS_FACING);
            if (frontFacing != null && frontFacing == CameraCharacteristics.LENS_FACING_FRONT) {
                int sensorOrientation = mCharacteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
                orientation = (orientation + sensorOrientation + 270) % 360;
            }
            mMediaRecorder.setOrientationHint(orientation);
            mMediaRecorder.prepare();
            mRequestBuilder.addTarget(mMediaRecorder.getSurface());
            targetSurfaces.add(mMediaRecorder.getSurface());

            if (null == mCameraDevice) {
                return;
            }

            // Prepare CameraCaptureSession
            mCameraDevice.createCaptureSession(targetSurfaces, new CameraCaptureSession.StateCallback() {
                @Override
                public void onConfigured(@NonNull CameraCaptureSession session) {
                    // The camera is already closed
                    if (null == mCameraDevice) {
                        return;
                    }
                    Log.e("CREATE_CAPTURE_SESSION", "record start!");
                    mMediaRecorder.start();
                    mCameraCaptureSession = session;
                    try {
                        // Now we can start capturing
                        CaptureRequest singleRequest = mRequestBuilder.build();
                        mCameraCaptureSession.setRepeatingRequest(singleRequest, mCaptureCallback, null);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }

                @Override
                public void onConfigureFailed(@NonNull CameraCaptureSession session) {
                    Log.e(TAG, "CreateCaptureSession onConfigureFailed!");
                }
            }, null);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void stopCamera() {
        try {
            if (null != mCameraCaptureSession) {
                mCameraCaptureSession.close();
                mCameraCaptureSession = null;
            }
            if (null != mCameraDevice) {
                mCameraDevice.close();
                mCameraDevice = null;
            }
            if (null != mImageReader) {
                mImageReader.close();
                mImageReader = null;
            }
            if (null != mMediaRecorder)
            {
                mMediaRecorder.stop();
                mMediaRecorder.reset();
                mMediaRecorder = null;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private CamcorderProfile getCamcorderProfile()
    {
        int width = mPreviewSize.getWidth();
        int height = mPreviewSize.getHeight();
        if (width == 720 && height == 480) {
            return CamcorderProfile.get(CamcorderProfile.QUALITY_480P);
        } else if (width == 1280 && height == 720) {
            return CamcorderProfile.get(CamcorderProfile.QUALITY_720P);
        } else if (width == 1920 && height == 1080) {
            return CamcorderProfile.get(CamcorderProfile.QUALITY_1080P);
        } else if (width == 3840 && height == 2160) {
            return CamcorderProfile.get(CamcorderProfile.QUALITY_2160P);
        } else {
            return CamcorderProfile.get(CamcorderProfile.QUALITY_720P);
        }
    }

    public String getVideoFile() {
        final File dcimFile = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM);
        final File camera2VideoImage = new File(dcimFile, getString(R.string.app_name));
        if (!camera2VideoImage.exists())
        {
            camera2VideoImage.mkdirs();
        }
        return String.format("%s/FrontCamera_%d.mp4", camera2VideoImage.getAbsolutePath(), System.currentTimeMillis());
    }

    public boolean hasHighSpeedCamcorder(Size size, int cameraCharacteristics) {
        if (size.getWidth() == 720 && size.getHeight() == 480) {
            return CamcorderProfile.hasProfile(cameraCharacteristics, CamcorderProfile.QUALITY_HIGH_SPEED_480P);
        } else if (size.getWidth() == 1280 && size.getHeight() == 720) {
            return CamcorderProfile.hasProfile(cameraCharacteristics, CamcorderProfile.QUALITY_HIGH_SPEED_720P);
        } else if (size.getWidth() == 1920 && size.getHeight() == 1080) {
            return CamcorderProfile.hasProfile(cameraCharacteristics, CamcorderProfile.QUALITY_HIGH_SPEED_1080P);
        } else if (size.getWidth() == 3840 && size.getHeight() == 2160) {
            return CamcorderProfile.hasProfile(cameraCharacteristics, CamcorderProfile.QUALITY_HIGH_SPEED_2160P);
        } else {
            return false;
        }
    }
}
