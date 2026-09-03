#!/usr/bin/env bash
set -euo pipefail

readonly TAPA_VERSION="0.1.20250815"
readonly ARCHIVE_NAME="tapa-${TAPA_VERSION}-xilinx-2024.2-linux-x86_64.tar.xz"
readonly ARCHIVE_SHA256="cc2efec778887676396f7513e94d8ef4c236e2ba42ffd4ed74bfdeceac130737"
readonly INSTALL_DIR="${HOME}/.rapidstream-tapa"
readonly EXPECTED_VERSION="tapa, version ${TAPA_VERSION}"

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
archive="${SCRIPT_DIR}/${ARCHIVE_NAME}"
path_line='export PATH="$HOME/.rapidstream-tapa/usr/bin:$PATH"'

if [[ -x "${INSTALL_DIR}/usr/bin/tapa" ]] &&
  [[ "$("${INSTALL_DIR}/usr/bin/tapa" --version)" == "${EXPECTED_VERSION}" ]]; then
  echo "${EXPECTED_VERSION} is already installed."
else
  if [[ -e "${INSTALL_DIR}" ]]; then
    echo "error: ${INSTALL_DIR} already exists but is not ${EXPECTED_VERSION}" >&2
    exit 1
  fi
  if [[ ! -f "${archive}" ]]; then
    echo "error: missing ${archive}" >&2
    exit 1
  fi

  printf '%s  %s\n' "${ARCHIVE_SHA256}" "${archive}" | sha256sum --check -
  staging=$(mktemp -d "${HOME}/.rapidstream-tapa.tmp.XXXXXX")
  trap 'rm -rf "${staging}"' EXIT
  tar -xJf "${archive}" -C "${staging}"

  actual_version=$("${staging}/usr/bin/tapa" --version)
  if [[ "${actual_version}" != "${EXPECTED_VERSION}" ]]; then
    echo "error: archive contains ${actual_version}, expected ${EXPECTED_VERSION}" >&2
    exit 1
  fi

  mv "${staging}" "${INSTALL_DIR}"
  trap - EXIT
fi

for profile in "${HOME}/.zshrc" "${HOME}/.bashrc"; do
  if [[ -f "${profile}" ]] && ! grep -Fqx -- "${path_line}" "${profile}"; then
    printf '\n%s\n' "${path_line}" >>"${profile}"
  fi
done

export PATH="${INSTALL_DIR}/usr/bin:${PATH}"
[[ "$(tapa --version)" == "${EXPECTED_VERSION}" ]]
echo "Installed ${EXPECTED_VERSION} in ${INSTALL_DIR}."
echo "Open a new terminal, or run: ${path_line}"
