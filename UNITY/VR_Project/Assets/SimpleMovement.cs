using System.Collections;
using System.Collections.Generic;
using UnityEngine;



public class SimpleMovement : MonoBehaviour
{
    public float speed = 5f;
    public float mouseSensitivity = 2f;
    public float playerHeight = 2f; // Height above ground
    public LayerMask groundLayer = -1; // What counts as ground

    private float xRotation = 0f;

    void Start()
    {
        Cursor.lockState = CursorLockMode.Locked;
    }

    void Update()
    {
        // Movement
        float horizontal = Input.GetAxis("Horizontal") * speed * Time.deltaTime;
        float vertical = Input.GetAxis("Vertical") * speed * Time.deltaTime;

        Vector3 direction = transform.right * horizontal + transform.forward * vertical;
        direction.y = 0;

        Vector3 newPosition = transform.position + direction;

        // RAYCAST to find ground height
        RaycastHit hit;
        if (Physics.Raycast(newPosition + Vector3.up * 100f, Vector3.down, out hit, 200f, groundLayer))
        {
            // Set position to ground height + player height
            newPosition.y = hit.point.y + playerHeight;
        }

        transform.position = newPosition;

        // Mouse Look
        float mouseX = Input.GetAxis("Mouse X") * mouseSensitivity;
        float mouseY = Input.GetAxis("Mouse Y") * mouseSensitivity;

        xRotation -= mouseY;
        xRotation = Mathf.Clamp(xRotation, -90f, 90f);

        transform.localRotation = Quaternion.Euler(xRotation, transform.localEulerAngles.y + mouseX, 0);
    }
}
