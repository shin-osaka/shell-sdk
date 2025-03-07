/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.

 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

package eggy.cocos2dx.lib;

import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Map;
import java.util.Queue;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

public class Cocos2dxDownloader {

    private int _id;
    private OkHttpClient _httpClient = null;

    private String _tempFileNameSuffix;
    private int _countOfMaxProcessingTasks;
    private final ConcurrentHashMap<Integer, Call> _taskMap = new ConcurrentHashMap<>();
    private final Queue<Runnable> _taskQueue = new LinkedList<>();
    private int _runningTaskCount = 0;
    private static final ConcurrentHashMap<String, Boolean> _resumingSupport = new ConcurrentHashMap<>();

    private void onProgress(final int id, final long downloadBytes, final long downloadNow, final long downloadTotal) {
        Cocos2dxHelper.runOnGLThread(new Runnable() {
            @Override
            public void run() {
                nativeOnProgress(_id, id, downloadBytes, downloadNow, downloadTotal);
            }
        });
    }

    private void onFinish(final int id, final int errCode, final String errStr, final byte[] data) {
        Call task = _taskMap.get(id);
        if (null == task)
            return;
        _taskMap.remove(id);
        _runningTaskCount -= 1;
        Cocos2dxHelper.runOnGLThread(new Runnable() {
            @Override
            public void run() {
                nativeOnFinish(_id, id, errCode, errStr, data);
            }
        });
        runNextTaskIfExists();
    }

    public static Cocos2dxDownloader createDownloader(int id, int timeoutInSeconds, String tempFileSuffix,
            int maxProcessingTasks) {
        Cocos2dxDownloader downloader = new Cocos2dxDownloader();
        downloader._id = id;

        if (timeoutInSeconds > 0) {
            downloader._httpClient = new OkHttpClient().newBuilder()
                    .followRedirects(true)
                    .followSslRedirects(true)
                    .callTimeout(timeoutInSeconds, TimeUnit.SECONDS)
                    .build();
        } else {
            downloader._httpClient = new OkHttpClient().newBuilder()
                    .followRedirects(true)
                    .followSslRedirects(true)
                    .build();
        }

        downloader._tempFileNameSuffix = tempFileSuffix;
        downloader._countOfMaxProcessingTasks = maxProcessingTasks;
        return downloader;
    }

    public static void createTask(final Cocos2dxDownloader downloader, int id_, String url_, String path_, String []header_) {
        final int id = id_;
        final String url = url_;
        final String path = path_;
        final String[] header = header_;

        Runnable taskRunnable = new Runnable() {
            String domain = null;
            String host = null;
            File tempFile = null;
            File finalFile = null;
            long downloadStart = 0;

            @Override
            public void run() {
                Call task = null;

                do {
                    if (path.length() > 0) {
                        try {
                            URI uri = new URI(url);
                            domain = uri.getHost();
                        } catch (URISyntaxException e) {
                            e.printStackTrace();
                            break;
                        } catch (NullPointerException e) {
                            e.printStackTrace();
                            break;
                        }

                        tempFile = new File(path + downloader._tempFileNameSuffix);
                        if (tempFile.isDirectory()) break;

                        File parent = tempFile.getParentFile();
                        if (!parent.isDirectory() && !parent.mkdirs()) break;

                        finalFile = new File(path);
                        if (finalFile.isDirectory()) break;
                        long fileLen = tempFile.length();

                        host = domain.startsWith("www.") ? domain.substring(4) : domain;
                        if (fileLen > 0) {
                            if (_resumingSupport.containsKey(host) && _resumingSupport.get(host)) {
                                downloadStart = fileLen;
                            } else {
                                try {
                                    PrintWriter writer = new PrintWriter(tempFile);
                                    writer.print("");
                                    writer.close();
                                }
                                catch (FileNotFoundException e) {
                                }
                            }
                        }
                    }

                    final Request.Builder builder = new Request.Builder().url(url);
                    for (int i = 0; i < header.length / 2; i++) {
                        builder.addHeader(header[i * 2], header[(i * 2) + 1]);
                    }
                    if (downloadStart > 0) {
                        builder.addHeader("RANGE", "bytes=" + downloadStart + "-");
                    }

                    final Request request = builder.build();
                    task = downloader._httpClient.newCall(request);
                    if (null == task) {
                        final String errStr = "Can't create DownloadTask for " + url;
                        Cocos2dxHelper.runOnGLThread(new Runnable() {
                            @Override
                            public void run() {
                                downloader.nativeOnFinish(downloader._id, id, 0, errStr, null);
                            }
                        });
                    } else {
                        downloader._taskMap.put(id, task);
                    }
                    task.enqueue(new Callback() {
                        @Override
                        public void onFailure(Call call, IOException e) {
                            downloader.onFinish(id, 0, e.toString(), null);
                        }

                        @Override
                        public void onResponse(Call call, Response response) throws IOException {
                            InputStream is = null;
                            byte[] buf = new byte[4096];
                            FileOutputStream fos = null;

                            try {

<<<<<<< HEAD
                                if (!(response.code() >= 200 && response.code() <= 206)) {
                                    // it is encourage to delete the tmp file when requested range not satisfiable.
                                    if (response.code() == 416) {
                                        File file = new File(path + downloader._tempFileNameSuffix);
                                        if (file.exists() && file.isFile()) {
                                            file.delete();
                                        }
                                    }
=======
                                if(response.code() != 200) {
>>>>>>> 80e06b1a6ce (feat: 实现NetDoctor测试网速)
                                    downloader.onFinish(id, -2, response.message(), null);
                                    return;
                                }

                                long total = response.body().contentLength();
                                if (path.length() > 0 && !_resumingSupport.containsKey(host)) {
                                    if (total > 0) {
                                        _resumingSupport.put(host, true);
                                    } else {
                                        _resumingSupport.put(host, false);
                                    }
                                }

                                long current = downloadStart;
                                is = response.body().byteStream();

                                if (path.length() > 0) {
                                    if (downloadStart > 0) {
                                        fos = new FileOutputStream(tempFile, true);
                                    } else {
                                        fos = new FileOutputStream(tempFile, false);
                                    }

                                    int len;
                                    while ((len = is.read(buf)) != -1) {
                                        current += len;
                                        fos.write(buf, 0, len);
                                        downloader.onProgress(id, len, current, total);
                                    }
                                    fos.flush();

                                    String errStr = null;
                                    do {
                                        if (finalFile.exists()) {
                                            if (finalFile.isDirectory()) {
                                                break;
                                            }
                                            if (!finalFile.delete()) {
                                                errStr = "Can't remove old file:" + finalFile.getAbsolutePath();
                                                break;
                                            }
                                        }
                                        tempFile.renameTo(finalFile);
                                    } while (false);

                                    if (errStr == null) {
                                        downloader.onFinish(id, 0, null, null);
                                        downloader.runNextTaskIfExists();
                                    }
                                    else
                                        downloader.onFinish(id, 0, errStr, null);
                                } else {
                                    ByteArrayOutputStream buffer;
                                    if(total > 0) {
                                        buffer = new ByteArrayOutputStream((int) total);
                                    } else {
                                        buffer = new ByteArrayOutputStream(4096);
                                    }

                                    int len;
                                    while ((len = is.read(buf)) != -1) {
                                        current += len;
                                        buffer.write(buf, 0, len);
                                        downloader.onProgress(id, len, current, total);
                                    }
                                    downloader.onFinish(id, 0, null, buffer.toByteArray());
                                    downloader.runNextTaskIfExists();
                                }
                            } catch (IOException e) {
                                e.printStackTrace();
                                downloader.onFinish(id, 0, e.toString(), null);
                            } finally {
                                try {
                                    if (is != null) {
                                        is.close();
                                    }
                                    if (fos != null) {
                                        fos.close();
                                    }
                                } catch (IOException e) {
                                }
                            }
                        }
                    });
                } while (false);
            }

    };downloader.enqueueTask(taskRunnable);}

    public static void abort(final Cocos2dxDownloader downloader, final int id) {
        Cocos2dxHelper.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Iterator iter = downloader._taskMap.entrySet().iterator();
                while (iter.hasNext()) {
                    Map.Entry entry = (Map.Entry) iter.next();
                    Object key = entry.getKey();
                    Call task = (Call) entry.getValue();
                    if (null != task && Integer.parseInt(key.toString()) == id) {
                        task.cancel();
                        downloader._taskMap.remove(id);
                        downloader.runNextTaskIfExists();
                        break;
                    }
                }
            }
        });
    }

    public static void cancelAllRequests(final Cocos2dxDownloader downloader) {
        Cocos2dxHelper.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                for (Object o : downloader._taskMap.entrySet()) {
                    Map.Entry entry = (Map.Entry) o;
                    Call task = (Call) entry.getValue();
                    if (null != task) {
                        task.cancel();
                    }
                }
            }
        });
    }

    private void enqueueTask(Runnable taskRunnable) {
        synchronized (_taskQueue) {
            if (_runningTaskCount < _countOfMaxProcessingTasks) {
                Cocos2dxHelper.getActivity().runOnUiThread(taskRunnable);
                _runningTaskCount++;
            } else {
                _taskQueue.add(taskRunnable);
            }
        }
    }

    private void runNextTaskIfExists() {
        synchronized (_taskQueue) {
            while (_runningTaskCount < _countOfMaxProcessingTasks &&
                    Cocos2dxDownloader.this._taskQueue.size() > 0) {

                Runnable taskRunnable = Cocos2dxDownloader.this._taskQueue.poll();
                Cocos2dxHelper.getActivity().runOnUiThread(taskRunnable);
                _runningTaskCount += 1;
            }
        }
    }

    native void nativeOnProgress(int id, int taskId, long dl, long dlnow, long dltotal);

    native void nativeOnFinish(int id, int taskId, int errCode, String errStr, final byte[] data);
}
