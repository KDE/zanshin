Feature: Available pages content
  As someone collecting notes
  I can see all the pages available to me
  In order to display them and add notes to them

  Scenario: Inbox and tags appear in the list
    Given I display the available pages
    When I list the items
    Then the list is:
       | display                           | icon                |
       | Inbox                             | mail-folder-inbox   |
       | Tags                              | folder              |
       | Tags / Philosophy                 | view-pim-tasks      |
       | Tags / Physics                    | view-pim-tasks      |

