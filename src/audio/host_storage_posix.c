#include "host_storage_posix.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "audio_decoder_registry.h"

static bool has_supported_extension(const char *path) {
  return audio_media_format_from_path(path) != AUDIO_MEDIA_FORMAT_UNKNOWN;
}

static bool join_path(char *out_path, size_t out_path_size, const char *lhs,
                      const char *rhs) {
  if (out_path == NULL || lhs == NULL || rhs == NULL) {
    return false;
  }
  const int written = snprintf(out_path, out_path_size, "%s/%s", lhs, rhs);
  return written > 0 && (size_t)written < out_path_size;
}

static bool host_storage_list_tracks(void *context, audio_track_t *out_tracks,
                                     size_t max_tracks, size_t *out_count) {
  if (context == NULL || out_tracks == NULL || out_count == NULL) {
    return false;
  }
  host_storage_context_t *host = (host_storage_context_t *)context;
  DIR *directory = opendir(host->root_directory);
  if (directory == NULL) {
    return false;
  }

  size_t track_count = 0;
  struct dirent *entry = NULL;
  while ((entry = readdir(directory)) != NULL) {
    if (track_count >= max_tracks) {
      break;
    }
    if (entry->d_name[0] == '.') {
      continue;
    }

    char full_path[AUDIO_TRACK_PATH_MAX] = {0};
    if (!join_path(full_path, sizeof(full_path), host->root_directory, entry->d_name)) {
      continue;
    }

    struct stat file_stat;
    if (stat(full_path, &file_stat) != 0 || !S_ISREG(file_stat.st_mode)) {
      continue;
    }
    if (!has_supported_extension(full_path)) {
      continue;
    }

    strncpy(out_tracks[track_count].path, full_path,
            sizeof(out_tracks[track_count].path) - 1);
    out_tracks[track_count].path[sizeof(out_tracks[track_count].path) - 1] = '\0';
    out_tracks[track_count].format = audio_media_format_from_path(full_path);
    track_count++;
  }
  closedir(directory);

  *out_count = track_count;
  return true;
}

static bool host_storage_open(void *context, const char *path,
                              audio_storage_file_t *out_file) {
  (void)context;
  if (path == NULL || out_file == NULL) {
    return false;
  }

  FILE *file = fopen(path, "rb");
  if (file == NULL) {
    return false;
  }
  if (fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    return false;
  }
  const long file_size = ftell(file);
  if (file_size < 0 || fseek(file, 0, SEEK_SET) != 0) {
    fclose(file);
    return false;
  }

  out_file->handle = file;
  out_file->size_bytes = (uint32_t)file_size;
  return true;
}

static size_t host_storage_read(void *context, audio_storage_file_t *file,
                                void *buffer, size_t byte_count) {
  (void)context;
  if (file == NULL || file->handle == NULL || buffer == NULL || byte_count == 0) {
    return 0;
  }
  return fread(buffer, 1, byte_count, (FILE *)file->handle);
}

static bool host_storage_seek(void *context, audio_storage_file_t *file,
                              uint32_t offset_bytes) {
  (void)context;
  if (file == NULL || file->handle == NULL) {
    return false;
  }
  return fseek((FILE *)file->handle, (long)offset_bytes, SEEK_SET) == 0;
}

static void host_storage_close(void *context, audio_storage_file_t *file) {
  (void)context;
  if (file == NULL || file->handle == NULL) {
    return;
  }
  fclose((FILE *)file->handle);
  file->handle = NULL;
  file->size_bytes = 0;
}

audio_storage_api_t host_storage_api = {
    .list_tracks = host_storage_list_tracks,
    .open = host_storage_open,
    .read = host_storage_read,
    .seek = host_storage_seek,
    .close = host_storage_close,
};

host_storage_context_t host_storage_create_context(const char *directory) {
  host_storage_context_t context = {
      .root_directory = directory,
  };
  return context;
}
